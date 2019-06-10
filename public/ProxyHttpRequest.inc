<?php

class ProxyHttpRequest extends http\Client\Request
{

    const POST_ENC_TYPE_APPLICATION = 'application/x-www-form-urlencoded';

    const POST_ENC_TYPE_MULTIPART = 'multipart/form-data';

    public function __construct()
    {
        parent::__construct($_SERVER['REQUEST_METHOD'], $this->getUrl(), 
            $this->getHeaders());
        
        if (isset($_POST)) {
            
            $post_data = $_POST;
            
            // If charset specified, convert back to upstream charset before adding.
            if (Conf::$default_upstream_charset) {
                array_walk($post_data, 
                    function (&$value)
                    {
                        $value = mb_convert_encoding($value, 
                            Conf::$default_upstream_charset, 'utf-8');
                    });
            }
            
            if ($this->getPostEncType() == self::POST_ENC_TYPE_MULTIPART) {
                
                // First unset the original content type header. addForm() will automatically add it
                // with it's own boundary value.
                $this->setHeader('Content-Type', NULL);
                
                $this->getBody()->addForm($_POST);
            } else {
                $this->getBody()->append(new http\QueryString($post_data));
            }
            
            Log::add($post_data, 'post_data');
        }
        
        $this->setOptions(
            [                
                'connecttimeout' => Conf::$proxy_http_request_connecttimeout,
                'dns_cache_timeout' => Conf::$proxy_http_request_dns_cache_timeout,
                'retrycount' => Conf::$proxy_http_request_retrycount,
                'timeout' => Conf::$proxy_http_request_timeout
            ]);
        
        Log::add($this->__toString(), 'ProxyHttpRequest->__toString()');
    }

    public function getHeaders()
    {
        $headers = getallheaders();
        
        $ignored_headers = array(

            // This only applies to the connection between the proxy and the user, 
            // not the proxy and the upstream origin.
            'Accept-Encoding',
            
            'Connection',
            'Content-Length',
            'Fastly-Client',
            'Fastly-Client-IP',
            'Fastly-FF',
            'Fastly-Orig-Host',
            'Fastly-SSL',
            'Host',
            'X-Forwarded-Host',
            'X-Forwarded-Server',
            'X-Varnish',
            
            // Otherwise CloudfFront to CloudFront requests are denied:
            'Via',
            'X-Amz-Cf-Id'
        );
        
        if (! Conf::$cookies_enabled) {
            $ignored_headers[] = 'Cookie';
        }
        
        foreach ($ignored_headers as $ignored_header) {
            if (isset($headers[$ignored_header])) {
                unset($headers[$ignored_header]);
            }
            
            $ignored_header_alt = strtolower($ignored_header);
            if (isset($headers[$ignored_header_alt])) {
                unset($headers[$ignored_header_alt]);
            }
        }
        
        foreach ($headers as $key => &$value) {
            TextExternalUrlFilters::applyReverse($value);
        }
        
        // Proxy standard headers.
        if (! isset($headers['X-Forwarded-For'])) {
            $headers['X-Forwarded-For'] = $_SERVER['REMOTE_ADDR'];
        }
        
        if (! isset($headers['X-Real-IP'])) {
            $real_ip = $headers['X-Forwarded-For'];
            
            // If multiple (command-separated) forwarded IPs, use the first one.
            if (strpos($real_ip, ',') !== false) {
                list ($real_ip) = explode(',', $real_ip);
            }
            
            $headers['X-Real-IP'] = $real_ip;
        }
        
        return $headers;
    }

    public function getUrl()
    {
        static $url;
        if (! isset($url)) {
            
            if (isset($_GET[RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME]) && $_GET[RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME] ==
                 Conf::OUTPUT_TYPE_APK && Conf::$apk_url) {
                
                $url = Conf::$apk_url;
                $filename = basename(parse_url($url, PHP_URL_PATH));
                header('Content-Disposition: attachment; filename=' . $filename);
                
                // Run after all other code to override other content-type header.
                register_shutdown_function(
                    function ()
                    {
                        header(
                            'Content-Type: application/vnd.android.package-archive');
                    });
            } else {
                $url = RedirectWhenBlockedFull::getRequestUriWithoutQueryStringParam();
                $this->removeThisScriptDirFromUrl($url);
                
                if (startsWith($url, '/http://') || startsWith($url, 
                    '/https://')) {
                    $url = substr($url, 1);
                    
                    if (! TextExternalUrlFilters::matchesUrl($url)) {
                        header('HTTP/1.0 403 Forbidden');
                        exit();
                    }
                    
                    // If we for some reason have the default upstream host and scheme in the URL, remove them.
                    $url_components = parse_url($url);
                    if ($url_components['host'] ==
                         Conf::getDefaultUpstreamBaseUrlComponent('host') && $url_components['scheme'] ==
                         Conf::getDefaultUpstreamBaseUrlComponent('scheme')) {
                        $new_url = http_build_path_query_fragment(
                            $url_components);
                        $new_url = RedirectWhenBlockedFull::getBaseUrl() .
                             ltrim($new_url, '/');
                        header('Location: ' . $new_url);
                        exit();
                    }
                    
                    // Use in DomUtlFilters for relative URLs.
                    $base_url_suffix = rtrim(http_build_scheme_host($url), '/') .
                         '/';
                    RedirectWhenBlockedFull::setBaseUrlSuffix($base_url_suffix);
                } else {
                    
                    if ($url == '/') {
                        if (Conf::$default_upstream_url) {
                            $url = Conf::$default_upstream_url;
                        }
                    }
                    $url = Conf::$default_upstream_base_url . $url;
                }
            }
        }
        
        // Reverse rewrites of parameters inside URL.
        TextExternalUrlFilters::applyReverse($url);
        Log::add($url, 'url');
        return $url;
    }

    public function getUrlComponent($name)
    {
        $components = $this->getUrlComponents();
        if (isset($components[$name])) {
            return $components[$name];
        }
    }

    public function getUrlComponents()
    {
        static $components;
        if (! isset($components)) {
            $components = parse_url($this->getUrl());
        }
        return $components;
    }

    private function getPostEncType()
    {
        $content_type = $this->getHeader('Content-Type');
        if ($content_type) {
            list ($enc_type) = explode(';', $content_type);
            if ($enc_type == self::POST_ENC_TYPE_MULTIPART) {
                return self::POST_ENC_TYPE_MULTIPART;
            }
        }
        
        // Default - used in most forms.
        return self::POST_ENC_TYPE_APPLICATION;
    }

    private function removeThisScriptDirFromUrl(&$url)
    {
        $this_script_dir = dirname($_SERVER['SCRIPT_NAME']);
        if ($this_script_dir != '/' &&
             substr($url, 0, strlen($this_script_dir)) == $this_script_dir) {
            $url = substr($url, strlen($this_script_dir));
        }
        return $url;
    }
}
