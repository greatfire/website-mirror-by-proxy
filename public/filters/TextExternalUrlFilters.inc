<?php

class TextExternalUrlFilters
{

    private static $hosts = array();

    public static function addHost($host)
    {
        if (! in_array($host, self::$hosts)) {
            self::$hosts[] = $host;
        }
    }

    public static function addHosts($hosts)
    {
        foreach ($hosts as $host) {
            self::addHost($host);
        }
    }

    public static function applyAll(&$body, $content_type = NULL)
    {
        foreach (self::getHostsFiltered() as $host) {
            
            $searches = array();
            
            // Convert protocol relative URLs to HTTPS.
            $searches['"//' . $host] = '"https://' . $host;
            $searches['\'//' . $host] = '\'https://' . $host;
            
            $urls = array();
            $urls[] = 'http://' . $host;
            $urls[] = 'https://' . $host;
            
            foreach ($urls as $url) {
                $searches[$url] = RedirectWhenBlockedFull::getBaseUrl() . $url;
            }
            
            // Because slashes are escaped in JSON, escape all search/replace patterns too.
            // JSON-type data may be embedded in JavaScript and HTML as well.
            if (in_array($content_type, 
                array(
                    ProxyHttpResponse::CONTENT_TYPE_JAVASCRIPT,
                    ProxyHttpResponse::CONTENT_TYPE_JSON,
                    ProxyHttpResponse::CONTENT_TYPE_TEXT_HTML
                ))) {
                
                $searches2 = array();
                
                foreach ($searches as $search => $replace) {
                    $search = str_replace('/', '\/', $search);
                    $replace = str_replace('/', '\/', $replace);
                    $searches2[$search] = $replace;
                }
                
                // For pure JSON content, use these searches only.
                if ($content_type == ProxyHttpResponse::CONTENT_TYPE_JSON) {
                    $searches = $searches2;
                }                 

                // Otherwise append.
                else {
                    $searches += $searches2;
                }
            }
            
            // Special for javascript (and HTML since it could be in a script tag).
            if (in_array($content_type, 
                array(
                    ProxyHttpResponse::CONTENT_TYPE_JAVASCRIPT,
                    ProxyHttpResponse::CONTENT_TYPE_TEXT_HTML
                ))) {
                foreach (array(
                    "'",
                    '"'
                ) as $quote) {
                    if ($host != Conf::getDefaultUpstreamBaseUrlComponent(
                        'host')) {
                        $prefix = 'http://' . $host;
                    } else {
                        $prefix = '';
                    }
                    $searches[$quote . $host . $quote] = $quote .
                         self::getBaseUrlHostAndPath() . $prefix . $quote;
                }
            }
            
            foreach ($searches as $search => $replace) {
                $body = str_ireplace($search, $replace, $body);
            }
        }
    }

    public static function applyReverse(&$body)
    {
        foreach (RedirectWhenBlockedFull::getAltBaseUrls() as $alt_base_url) {
            
            $searches = array(
                $alt_base_url . 'http://' => 'http://',
                $alt_base_url . 'https://' => 'https://',
                $alt_base_url => Conf::getDefaultUpstreamBaseUrlComponent(
                    'scheme') . '://' .
                     Conf::getDefaultUpstreamBaseUrlComponent('host') . '/'
            );
            
            // Add url-encoded versions.
            foreach ($searches as $search => $replace) {
                $search = urlencode($search);
                if (! isset($searches[$search])) {
                    $searches[$search] = urlencode($replace);
                }
            }
            
            // Special case when only first colon is encoded.
            // Should be moved to site-specific config.
            foreach ($searches as $search => $replace) {
                $search = str_replace('://', urlencode(':') . '//', $search);
                if (! isset($searches[$search])) {
                    $searches[$search] = str_replace('://', 
                        urlencode(':') . '//', $replace);
                }
            }
            
            foreach ($searches as $search => $replace) {
                $body = str_replace($search, $replace, $body);
            }
        }
    }

    public static function matchesUrl($url)
    {
        $host = parse_url($url, PHP_URL_HOST);
        if (in_array($host, self::$hosts)) {
            return true;
        }
    }

    private static function getBaseUrlHostAndPath()
    {
        static $host_and_path;
        if (! isset($host_and_path)) {
            $base_url_components = parse_url(
                RedirectWhenBlockedFull::getBaseUrl());
            $host_and_path = $base_url_components['host'];
            if (isset($base_url_components['path'])) {
                $host_and_path .= $base_url_components['path'];
            }
        }
        return $host_and_path;
    }

    // Remove hosts which start with other hosts (eg remove google.com.hk if google.com is in the list).
    // Otherwise some hosts may be replaced twice.
    private static function getHostsFiltered()
    {
        static $filtered_hosts;
        if (! isset($filtered_hosts)) {
            $filtered_hosts = array();
            sort(self::$hosts);
            foreach (self::$hosts as $host) {
                foreach ($filtered_hosts as $filtered_host) {
                    if (startsWith($host, $filtered_host)) {
                        continue 2;
                    }
                }
                $filtered_hosts[] = $host;
            }
        }
        return $filtered_hosts;
    }
}