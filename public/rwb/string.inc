<?php

function startsWith($str, $needle)
{
    if (is_array($needle)) {
        foreach ($needle as $n) {
            if (startsWith($str, $n)) {
                return true;
            }
        }
        return false;
    }

    if (strlen($needle) > strlen($str)) {
        return false;
    }

    return strcasecmp(substr($str, 0, strlen($needle)), $needle) === 0;
}