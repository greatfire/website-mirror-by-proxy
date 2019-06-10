<?php

class CurlTest extends StatusTest
{

	function run()
	{
		// Test HTTP response code.
		$url = 'http://' . $_SERVER['HTTP_HOST'];
		$ch = curl_init($url);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
		curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
		$result = curl_exec($ch);
		$http_status = curl_getinfo($ch, CURLINFO_HTTP_CODE);
		curl_close($ch);
		if ($http_status == 200) {
			$this->messages[] = "$url passed ($http_status)";
			return true;
		} else {
			$this->n_failed ++;
			$this->messages[] = "$url failed ($http_status)";
		}
	}
}