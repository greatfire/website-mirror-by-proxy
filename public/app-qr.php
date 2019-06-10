<?php
$apk_url = RedirectWhenBlockedFull::getBaseUrl() . '?' .
     RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME . '=' .
     Conf::OUTPUT_TYPE_APK;

$url = 'https://chart.googleapis.com/chart?chs=200x200&cht=qr&chl=' . urlencode(
    $apk_url) . '&choe=UTF-8';
$ch = curl_init($url);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
$response = curl_exec($ch);
curl_close($ch);

if ($response) {
    header(
        'Cache-Control: ' .
             getCacheControlHeader(60 * 60 * 24, 60 * 60 * 24 * 7, 
                60 * 60 * 24 * 7));
    header('Content-Type: image/png');
    print $response;
}