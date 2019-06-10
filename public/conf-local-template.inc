<?php

/*
 * Modify as needed, and rename this file to conf-local.inc.
 */

// The main URL you want to proxy.
Conf::$default_upstream_base_url = '';

// Third party hosts which should also be proxied (eg example.com).
TextExternalUrlFilters::addHosts(
	array(
		
	));

// To get javascript to work you may have to manually rewrite some text.
//TextInternalUrlFilters::addSearch('search', 'replace');

// For debugging. Make sure a directory called 'log' exists and is writable.
Log::enable();