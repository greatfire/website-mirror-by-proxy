<?php

class Conf
{
    
    // Using the same QUERY_STRING_PARAM_NAME as in RWB so make sure this value isn't used by RWB.
    const OUTPUT_TYPE_ALT_BASE_URLS = 10;

    const OUTPUT_TYPE_APK_URLS = 11;

    const OUTPUT_TYPE_APP_URLS = 13;

    const OUTPUT_TYPE_STATUS = 12;

    const OUTPUT_TYPE_APK = 'apk';

    const OUTPUT_TYPE_APP_QR = 'app-qr.png';

    const OUTPUT_TYPE_APP = 'app';

    public static $alt_base_urls_key;
    
    // 1 day.
    public static $alt_base_urls_modified_ago_max = 86400;

    public static $apk_url;

    public static $china_ip_for_dns_poisoning_test;

    public static $cookies_enabled = false;

    public static $default_cache_control_max_age = 60;
    
    // 1 day.
    public static $default_cache_control_stale_if_error = 86400;

    public static $default_cache_control_stale_while_revalidate = 600;

    public static $default_upstream_base_url;
    
    // This should only be used if upstream server isn't configured correctly. Will override server responses.
    public static $default_upstream_charset;

    public static $default_upstream_url;

    public static $html_body_appendix = '';

    public static $html_head_appendix = '';

    public static $proxy_http_request_connecttimeout = 10;

    public static $proxy_http_request_dns_cache_timeout = 60;

    public static $proxy_http_request_retrycount = 1;

    public static $proxy_http_request_timeout = 30;

    public static $translatable_text = array(
        'downloading' => 'Downloading {{APK}} ...',
        'scan_or_click' => 'Scan or click on your Android device to download the app.',
        'weixin' => 'To download {{APK}} click the top right button and select "Open in a browser"'
    );

    public static function addGoogleAnalyticsId($id)
    {
        static $i;
        if (! isset($i)) {
            $i = 0;
        }
        $i ++;
        
        $name = 'wmbp' . $i;
        $ga_script = <<<EOF
<script>
  (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
  (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
  m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
  })(window,document,'script','//www.google-analytics.com/analytics.js','ga');

  ga('create', '$id', 'auto', {'name': '$name'});
  ga('$name.send', 'pageview');

</script>
EOF;
        // Appends to main page / iframe container.
        RedirectWhenBlockedFull::appendToHtmlBody($ga_script);
        
        // Appends to iframed content.
        self::appendToHtmlBody($ga_script);
    }

    public static function appendToHtmlBody($str)
    {
        self::$html_body_appendix .= $str;
    }

    public static function appendToHtmlHead($str)
    {
        self::$html_head_appendix .= $str;
    }

    public static function getDefaultUpstreamBaseUrlComponent($name)
    {
        $components = self::getDefaultUpstreamBaseUrlComponents();
        if (isset($components[$name])) {
            return $components[$name];
        }
    }

    public static function getDefaultUpstreamBaseUrlComponents()
    {
        static $components;
        if (! isset($components)) {
            $components = parse_url(self::$default_upstream_base_url);
        }
        return $components;
    }
}