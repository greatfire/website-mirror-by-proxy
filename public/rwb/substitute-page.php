<!DOCTYPE html>
<html
	manifest="<?php print $rwb_path_relative_to_request_path ?>/rwb.appcache?hash=<?php print rwb_appcache_get_hash() ?>">
<head>
<meta charset="UTF-8">
<meta name="viewport"
	content="width=device-width, initial-scale=1, user-scalable=yes">
<title><?php print self::$website_title ?></title>
<?php print '<style>'; require 'substitute-page.css'; print '</style>'; ?>
<script>
if (top != self && window.name == '<?php print self::IFRAME_WINDOW_NAME ?>') {
	console.log('top != self');
	top.location = window.location;
}
</script>
</head>
<body>
	<div>
		<p><?php print self::getText('loading') ?></p>
		<h1><?php print self::$website_title ?></h1>
		<p><?php print self::getText('if_website_fails') ?></p>
		<?php if(self::$alt_url_collections) { ?>
		<ul>
			<?php foreach(self::$alt_url_collections as $alt_url_collection) { ?>
			<li><a href="<?php print $alt_url_collection; ?>" target="_blank"><?php print $alt_url_collection; ?></a></li>
			<?php } ?>
		</ul>
		<?php } ?>
	</div>
	<iframe frameBorder="0" scrolling="no" verticalscrolling="no"
		name="<?php print self::IFRAME_WINDOW_NAME ?>" seamless="seamless"
		height="600px" src="<?php print htmlspecialchars($iframe_src) ?>"></iframe>
	<script
		src="<?php print $rwb_path_relative_to_request_path ?>/jquery-1.11.1.min.js"></script>
	<script>
	var get_param_name = '<?php print self::QUERY_STRING_PARAM_NAME ?>';
	var jsonp_callback_basename = '<?php print self::JSONP_CALLBACK_BASENAME ?>';
	var output_type_iframe = '<?php print self::OUTPUT_TYPE_IFRAME ?>';
	var output_type_jsonp = '<?php print self::OUTPUT_TYPE_JSONP ?>';
	if (top != self) {
		var new_location = window.location.href;
		new_location += new_location.indexOf('?') == -1 ? '?' : '&';
		new_location += get_param_name + '=' + output_type_iframe;
		console.log('changing ' + window.location.href + ' to ' + new_location);
		window.location = new_location;
	} else {
    	window.name = '<?php print self::TOP_WINDOW_NAME ?>';
    	var alt_base_urls = <?php print json_encode(self::$alt_base_urls) ?>;
    	var if_website_fails_top = <?php print json_encode(self::getText('if_website_fails_top')) ?>;
    	<?php print str_replace("\n", "\n\t\t", file_get_contents(__DIR__ . '/substitute-page.js')) . "\n"; ?>
	}
	</script>
	<?php print self::$html_body_appendix ?>
</body>
</html>