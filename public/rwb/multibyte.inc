<?php

function mb_convert_encoding_plus($str, $to_encoding, $from_encoding)
{
	$converted = null;
	
	if(!$to_encoding || !$from_encoding) {
		return $str;
	}
	
	if (mb_supports_encoding($from_encoding) &&
		 mb_supports_encoding($to_encoding)) {
		$converted = mb_convert_encoding($str, $to_encoding, $from_encoding);
	}
	
	if(!$converted) {
		$converted = @iconv($from_encoding, $to_encoding . '//TRANSLIT', $str);
	}

	if(!$converted) {
		$converted = iconv($from_encoding, $to_encoding . '//IGNORE', $str);
	}
	
	if(!$converted) {
	    trigger_error("Failed to convert from $from_encoding to $to_encoding");
	}
	
	return $converted;
}

function mb_supports_encoding($encoding)
{
	static $encodings;
	if (! isset($encodings)) {
		$encodings = array();
		foreach (mb_list_encodings() as $e) {
			$encodings[] = strtolower($e);
		}
		
		// These are listed but under different names. Add the standard names (see https://bugs.php.net/bug.php?id=45086).
		$encodings[] = 'gbk';
		$encodings[] = 'gb2312';
	}
	
	$encoding = strtolower($encoding);
	
	if (in_array($encoding, $encodings)) {
		return true;
	}
}