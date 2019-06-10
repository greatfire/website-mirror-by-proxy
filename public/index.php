<?php
require 'main.inc';

// Default cache.
// Will be overwritten by message below if it has it's own Cache-Control header.
// Send this early, to prevent caching error pages for longer than the duration.
header(
    'Cache-Control: ' . getCacheControlHeader(
        Conf::$default_cache_control_max_age, 
        Conf::$default_cache_control_stale_while_revalidate, 
        Conf::$default_cache_control_stale_if_error));

// Make sure to send these security headers are included in all responses.
$required_security_headers['X-Content-Type-Options'] = 'nosniff';
$required_security_headers['X-Download-Options'] = 'noopen';
$required_security_headers['X-XSS-Protection'] = '1; mode=block';

// HSTS disabled for now.
// $required_security_headers['Strict-Transport-Security'] = 'max-age=31536000; includeSubdomains';

foreach ($required_security_headers as $key => $value) {
    header($key . ': ' . $value);
}

Log::add($_SERVER, '$_SERVER');

if (isset($_GET[RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME])) {
    
    // Key cannot be empty.
    if (Conf::$alt_base_urls_key) {
        
        // Verify key. Set this in conf-local.inc.
        if (isset($_GET['key']) && $_GET['key'] == Conf::$alt_base_urls_key) {
            
            if ($_GET[RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME] ==
                 Conf::OUTPUT_TYPE_ALT_BASE_URLS) {
                header('Content-Type: application/javascript');
                print json_encode(RedirectWhenBlockedFull::getAltBaseUrls());
                exit();
            }
            
            if ($_GET[RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME] ==
                 Conf::OUTPUT_TYPE_APK_URLS) {
                header('Content-Type: application/javascript');
                $urls = array();
                foreach (RedirectWhenBlockedFull::getAltBaseUrls() as $url) {
                    $url .= '?' .
                         RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME . '=' .
                         Conf::OUTPUT_TYPE_APK;
                    $urls[] = $url;
                }
                print json_encode($urls);
                exit();
            }
            
            if ($_GET[RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME] ==
                 Conf::OUTPUT_TYPE_APP_URLS) {
                header('Content-Type: application/javascript');
                $urls = array();
                foreach (RedirectWhenBlockedFull::getAltBaseUrls() as $url) {
                    $url .= '?' .
                         RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME . '=' .
                         Conf::OUTPUT_TYPE_APP;
                    $urls[] = $url;
                }
                print json_encode($urls);
                exit();
            }
            
            if ($_GET[RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME] ==
                 Conf::OUTPUT_TYPE_STATUS) {
                header('Cache-Control: max-age=0');
                header('Content-Type: text/plain');
                require 'status_tests/StatusTest.inc';
                
                foreach (scandir('status_tests/enabled') as $file) {
                    if ($file[0] == '.') {
                        continue;
                    }
                    require 'status_tests/enabled/' . $file;
                    $class = basename($file, '.inc');
                    $test = new $class();
                    $tests[] = $test;
                }
                
                foreach ($tests as $test) {
                    if (! $test->passed()) {
                        http_response_code(503);
                        break;
                    }
                }
                
                foreach ($tests as $i => $test) {
                    if ($i > 0) {
                        print "\n---\n";
                    }
                    print $test;
                }
                
                exit();
            }
        }
    }
    
    if (Conf::$apk_url) {
        if ($_GET[RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME] ==
             Conf::OUTPUT_TYPE_APP) {
            require 'app.php';
            exit();
        }
        
        if ($_GET[RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME] ==
             Conf::OUTPUT_TYPE_APP_QR) {
            require 'app-qr.php';
            exit();
        }
    }
}

$request = new ProxyHttpRequest();

// Hijack crossdomain.xml.
if ($request->getUrlComponent('path') == '/crossdomain.xml' &&
     getDownstreamOrigin()) {
    header('Content-Type: application/xml');
    $downstream_origin = getDownstreamOrigin();
    print 
        <<<EOF
<?xml version="1.0" ?>
<cross-domain-policy>
  <site-control permitted-cross-domain-policies="master-only"/>
  <allow-access-from domain="$downstream_origin"/>
  <allow-http-request-headers-from domain="$downstream_origin" headers="*"/>
</cross-domain-policy>
EOF;
    exit();
}

$client = new http\Client();
$client->setOptions(
    [
        'connecttimeout' => Conf::$proxy_http_request_connecttimeout,
        'dns_cache_timeout' => Conf::$proxy_http_request_dns_cache_timeout,
        'retrycount' => Conf::$proxy_http_request_retrycount,
        'timeout' => Conf::$proxy_http_request_timeout
    ]);
$client->enqueue($request)->send();
$response = new ProxyHttpResponse($client->getResponse(), $request);

$body = $response->getBody();
$headers = $response->getHeaders();

// Default - can be overriden below.
$headers['X-Frame-Options'] = 'SAMEORIGIN';

if (getDownstreamOrigin()) {
    $headers['Access-Control-Allow-Origin'] = getDownstreamOrigin();
    
    // See http://stackoverflow.com/questions/12409600/error-request-header-field-content-type-is-not-allowed-by-access-control-allow.
    $headers['Access-Control-Allow-Headers'] = 'Origin, X-Requested-With, Content-Type, Accept';
    
    $headers['X-Frame-Options'] = 'ALLOW-FROM ' . getDownstreamOrigin();
}

header($response->getResponseInfo());
foreach ($headers as $key => $values) {
    
    // Don't overwrite security headers.
    if (isset($required_security_headers[$key])) {
        continue;
    }
    
    if (! is_array(($values))) {
        $values = array(
            $values
        );
    }
    
    foreach ($values as $i => $value) {
        header($key . ': ' . $value, ($i == 0));
    }
}

print $body;