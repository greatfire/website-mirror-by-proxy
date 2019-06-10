<?php
require 'multibyte.inc';
require 'rwb.appcache.functions.inc';
require 'string.inc';

class RedirectWhenBlockedFull
{
    
    // This can't be used elsewhere on the website.
    const QUERY_STRING_PARAM_NAME = 'rwb3498472';
    
    // Same as above, used for anchors targeting top window.
    const TOP_WINDOW_NAME = self::QUERY_STRING_PARAM_NAME;
    
    // Used to identify main iframe.
    const IFRAME_WINDOW_NAME = 'rwb3498472i';

    const JSONP_CALLBACK_BASENAME = 'jsonpCallback';

    const OUTPUT_TYPE_IFRAME = 1;

    const OUTPUT_TYPE_JSONP = 2;

    public static $translatable_text = array(
        'loading' => 'Loading...',
        'if_website_fails' => 'If the website fails to load, you may be able to find another
			mirror URL here:',
        'if_website_fails_top' => 'If the website fails to load, you may be able to find another
			mirror URL on {{alt_url_collection_links}}.',
        'or' => 'or'
    );

    private static $alt_base_urls = array(), $alt_url_collections = array();
    
    // Manually set charset.
    private static $charset;
    
    // If a part of the website uses a separate base URL.
    private static $base_url_suffix = '';
    
    // Shown on the cached page while the content is loading.
    private static $html_body_appendix = '';

    private static $website_title;

    public static function addAltBaseUrl($alt_base_url)
    {
        self::$alt_base_urls[] = $alt_base_url;
    }

    public static function addAltBaseUrls($alt_base_urls)
    {
        foreach ($alt_base_urls as $alt_base_url) {
            self::addAltBaseUrl($alt_base_url);
        }
    }

    public static function addAltUrlCollection($alt_url_collection)
    {
        self::$alt_url_collections[] = $alt_url_collection;
    }

    public static function addAltUrlCollections($alt_url_collections)
    {
        foreach ($alt_url_collections as $alt_url_collection) {
            self::addAltUrlCollection($alt_url_collection);
        }
    }

    public static function addUrlsFromConfDir()
    {
        $alt_base_urls = file(__DIR__ . '/conf/alt_base_urls.txt', 
            FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        self::addAltBaseUrls($alt_base_urls);
        $alt_url_collections = file(__DIR__ . '/conf/alt_url_collections.txt', 
            FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        self::addAltUrlCollections($alt_url_collections);
    }

    public static function appendToHtmlBody($str)
    {
        self::$html_body_appendix .= $str;
    }

    public static function getAltBaseUrls()
    {
        return self::$alt_base_urls;
    }

    public static function getBaseTag()
    {
        return '<base href="' . RedirectWhenBlockedFull::getBaseUrl(true) .
             '" target="' . self::TOP_WINDOW_NAME . '">';
    }
    
    // Guess the current base URL.
    public static function getBaseUrl($append_suffix = false)
    {
        $candidates = array();
        
        if (isset($_GET[self::QUERY_STRING_PARAM_NAME])) {
            $url = substr($_GET[self::QUERY_STRING_PARAM_NAME], 1);
            $candidates[] = $url;
        }
        
        if (isset($_SERVER['HTTP_ORIGIN'])) {
            $candidates[] = $_SERVER['HTTP_ORIGIN'];
        }
        
        if (isset($_SERVER['HTTP_REFERER'])) {
            $candidates[] = $_SERVER['HTTP_REFERER'];
        }
        
        // Default fallback.
        $base_url = self::$alt_base_urls[0];
        
        foreach ($candidates as $candidate) {
            foreach (self::$alt_base_urls as $alt_base_url) {
                if ($candidate && startsWith($candidate, $alt_base_url)) {
                    $base_url = $alt_base_url;
                }
            }
        }
        
        if ($append_suffix) {
            $base_url .= self::$base_url_suffix;
        }
        
        return $base_url;
    }

    public static function getOutputType()
    {
        static $output_type;
        
        if (! isset($output_type)) {
            $output_type = isset($_GET[self::QUERY_STRING_PARAM_NAME]) ? $_GET[self::QUERY_STRING_PARAM_NAME][0] : NULL;
        }
        
        return $output_type;
    }
    
    // Because we append a custom query string parameter, the script may want to access
    // the "original" URI, with the custom query string parameter removed.
    public static function getRequestUriWithoutQueryStringParam()
    {
        $uri = $_SERVER['REQUEST_URI'];
        if (! isset($_GET[self::QUERY_STRING_PARAM_NAME])) {
            return $uri;
        }
        
        $get_copy = $_GET;
        unset($get_copy[self::QUERY_STRING_PARAM_NAME]);
        
        // If JSONP, remove other parameters added by jQuery.
        if (self::getOutputType() == self::OUTPUT_TYPE_JSONP) {
            if (isset($get_copy['callback'])) {
                unset($get_copy['callback']);
            }
            if (isset($get_copy['_'])) {
                unset($get_copy['_']);
            }
        }
        
        $uri_components = parse_url($uri);
        $uri = $uri_components['path'];
        
        if ($get_copy) {
            $uri .= '?' . http_build_query($get_copy);
        }
        
        if (isset($uri_components['fragment'])) {
            $uri .= '#' . $uri_components['fragment'];
        }
        
        return $uri;
    }

    public static function injectBaseTag(&$html)
    {
        // Insert base tag, unless there already is one.
        if (stripos($html, '<base') === false) {
            
            $base_tag = self::getBaseTag();
            
            // Varieties of head tag. Some sites use the second version...
            $head_tags = array(
                '<head>',
                '<head >',
                
                // If no head tag, insert after body tag instead. Yes, this means the HTML is invalid. But
                // adding a head tag to the body segment works too.
                '<body>',
                '<body >'
            );
            
            $base_tag_injected = false;
            foreach ($head_tags as $head_tag) {
                $html = str_replace($head_tag, $head_tag . $base_tag, $html, 
                    $count);
                if ($count > 0) {
                    $base_tag_injected = true;
                    break;
                }
            }
            
            // For even more invalid HTML (no head tag, no body tag), just append base tag to start of doc.
            if (! $base_tag_injected) {
                $html = $base_tag . "\n" . $html;
            }
        }
    }
    
    // Main activity.
    public static function run()
    {
        /*
         * Normal request. Substitute the response with our own page.
         */
        if (! isset($_GET[self::QUERY_STRING_PARAM_NAME])) {
            
            $iframe_src = $_SERVER['REQUEST_URI'];
            if ($_SERVER['QUERY_STRING']) {
                $iframe_src .= '&';
            } else {
                $iframe_src .= '?';
            }
            
            $iframe_src .= self::QUERY_STRING_PARAM_NAME . '=' .
                 self::OUTPUT_TYPE_IFRAME;
            
            $request_path_depth = count(
                explode('/', parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH)));
            $script_path_depth = count(explode('/', $_SERVER['SCRIPT_NAME']));
            $rwb_path_relative_to_request_path = str_repeat('../', 
                $request_path_depth - $script_path_depth) . 'rwb';
            
            require 'substitute-page.php';
            exit();
        }
        
        if (self::getOutputType() == self::OUTPUT_TYPE_JSONP) {
            
            // Output header now since other header output might block it later.
            header('Content-Type: application/javascript');
        }         
        
        // Turn on output buffer to capture all output.
        ob_start();
        
        // Make this run after all output is completed.
        register_shutdown_function(
            function ()
            {
                
                $html = ob_get_clean();
                
                RedirectWhenBlockedFull::injectBaseTag($html);
                
                /*
                 * This request comes from another base url (mirror or source host).
                 * We take the normal output and turn it into a jsonp response.
                 */
                if (RedirectWhenBlockedFull::getOutputType() ==
                     RedirectWhenBlockedFull::OUTPUT_TYPE_JSONP) {
                    
                    print 
                        self::getJsonpCallbackName() . '(' . json_encode(
                            array(
                                'html' => mb_convert_encoding_plus($html, 
                                    'UTF-8', 
                                    RedirectWhenBlockedFull::getCharset($html))
                            )) . ')';
                } 

                else {
                    print $html;
                }
            });
    }

    public static function setBaseUrlSuffix($base_url_suffix)
    {
        self::$base_url_suffix = $base_url_suffix;
    }

    public static function setCharset($charset)
    {
        self::$charset = $charset;
    }

    public static function setWebsiteTitle($website_title)
    {
        self::$website_title = $website_title;
    }

    private static function getCharset($html)
    {
        if (isset(self::$charset) && self::$charset) {
            return self::$charset;
        }
        
        if (preg_match('/charset=([^\'"]+)/', $html, $match)) {
            return $match[1];
        }
    }

    private static function getJsonpCallbackName()
    {
        if (isset($_GET['callback'])) {
            if (preg_match('/^' . self::JSONP_CALLBACK_BASENAME . '\d+$/', 
                $_GET['callback'])) {
                return $_GET['callback'];
            }
        }
        
        // callback argument missing or not valid. fallback to default one.
        return self::JSONP_CALLBACK_BASENAME . '1';
    }
    
    private static function getText($key) {        
        if(!isset(self::$translatable_text[$key])) {
            return '';
        }
        
        $text = self::$translatable_text[$key];
        
        // Magic replacements.
        if(strpos($text, '{{alt_url_collection_links}}') !== false) {
            $alt_url_collection_links = array();
            foreach(self::$alt_url_collections as $url) {
                $domain = parse_url($url, PHP_URL_HOST);
                list($domain_without_tld) = explode('.', $domain, 2);
                $alt_url_collection_links[] = '<a href="' . $url . '" target="_blank">' . $domain_without_tld . '</a>';
            }
            $alt_url_collection_links_str = implode(' ' . self::$translatable_text['or'] . ' ', $alt_url_collection_links);
            $text = str_replace('{{alt_url_collection_links}}', $alt_url_collection_links_str, $text);
        }
        
        return $text;
    }
}