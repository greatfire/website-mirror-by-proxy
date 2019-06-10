<?php
if (! function_exists('getallheaders')) {

    function getallheaders()
    {
        $headers = array();
        foreach ($_SERVER as $name => $value) {
            if (substr($name, 0, 5) == 'HTTP_') {
                $headers[str_replace(' ', '-', 
                    ucwords(strtolower(str_replace('_', ' ', substr($name, 5)))))] = $value;
            }
        }
        return $headers;
    }
}

function http_add_query_component($url, $key, $value)
{
    if (strpos($url, '?') === FALSE) {
        $url .= '?';
    } else {
        $url .= '&';
    }
    $url .= $key . '=' . urlencode($value);
    return $url;
}

function http_build_scheme_host($mixed)
{
    if (is_array($mixed)) {
        $components = $mixed;
    } else {
        $components = parse_url($mixed);
    }
    
    $scheme_and_host = $components['scheme'] . '://';
    $scheme_and_host .= $components['host'];
    return $scheme_and_host;
}

function http_build_scheme_host_directory($url)
{
    $components = parse_url($url);
    $scheme_host_directory = http_build_scheme_host($components);
    if (isset($components['path'])) {
        $scheme_host_directory .= dirname($components['path']);
    }
    return $scheme_host_directory;
}

function http_build_path_query_fragment($mixed)
{
    if (is_array($mixed)) {
        $components = $mixed;
    } else {
        $components = parse_url($mixed);
    }
    
    $url = '';
    if (isset($components['path'])) {
        $url .= $components['path'];
    }
    if (isset($components['query'])) {
        $url .= '?' . $components['query'];
    }
    if (isset($components['fragment'])) {
        $url .= '#' . $components['fragment'];
    }
    return $url;
}