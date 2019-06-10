<?php
$apk_url = './?' .
     RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME . '=' .
     Conf::OUTPUT_TYPE_APK;

$apk_filename = basename(parse_url(Conf::$apk_url, PHP_URL_PATH));

$app_url = './?' .
     RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME . '=' .
     Conf::OUTPUT_TYPE_APP;

$app_qr_url = './?' .
     RedirectWhenBlockedFull::QUERY_STRING_PARAM_NAME . '=' .
     Conf::OUTPUT_TYPE_APP_QR;
?>
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport"
	content="width=device-width, initial-scale=1, user-scalable=yes">
<title><?php print $apk_filename ?></title>
<style>
body {
    line-height: 150%;
    margin: 0;
	text-align: center;
	word-wrap: break-word;
}

a {
	text-decoration: none;
}

#main {
	border: 10px solid lightgrey;
	border-radius: 5px;
	max-width: 500px;
	padding: 8px;
}

#dl, #weixin {
	font-weight: bold;
	margin-bottom: 20px;
}

#qr img {
    margin-bottom: -20px;
}
</style>
</head>
<body>
	<div id="main">
		<div id="dl" style="display: none">
			<p><?php print str_replace('{{APK}}', $apk_filename, Conf::$translatable_text['downloading']) ?></p>
			<img src="rwb/spinner.gif" />
		</div>
		<div id="weixin" style="display: none">
			<p><?php print str_replace('{{APK}}', $apk_filename, Conf::$translatable_text['weixin']) ?></p>
		</div>
		<div id="qr">
			<a href="<?php print $apk_url ?>"> <img
				src="<?php print $app_qr_url ?>" /> <br />
            <?php print Conf::$translatable_text['scan_or_click']; ?>
        </a>
		</div>
	</div>
	<script src="rwb/jquery-1.11.1.min.js"></script>
	<script type="text/javascript">
    function dl() {
        $('#dl').show();
        $('#qr').fadeTo('slow', 0.4);
        setTimeout(function() {
            location.href = $('a').attr('href');
        }, 300);
    }
    
    var testAndroid = /(android)/ig;
    var testIos = /(iphone|ipad|ipod)/ig;
    var testWeixin = /(micromessenger)/ig;
    var isAndroid = testAndroid.test(navigator.userAgent);
    var isIos = testIos.test(navigator.userAgent);
    var isWeixin = testWeixin.test(navigator.userAgent);
    if(isWeixin) {
        $('#weixin').show();
        $('#qr').fadeTo('slow', 0.4);
    }
    else if(isAndroid) {
    	dl();
    }
    </script>
</body>
</html>