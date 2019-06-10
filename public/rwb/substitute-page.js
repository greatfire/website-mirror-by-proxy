var done = false;

function iframeLoaded($iframe) {
	if($iframe.data('hasLoaded') === true) {
		console.log('iframe already loaded');
		return;
	}
	
	done = true;
	
	console.log('iframe loaded');
	$iframe.data('hasLoaded', true);
	$('div').html(if_website_fails_top);
	$('div').addClass('top');

	manageIframe($iframe);
	setInterval(function() {
		try {
			manageIframe($iframe);
		} catch (err) {
		}
	}, 1000);
}

function manageIframe($iframe) {	
	if($iframe.contents().find('title').text()) {
		document.title = $iframe.contents().find('title').text();
	}

	if($iframe.contents().find('head').length > 0 && $iframe.contents().find('head base').length == 0) {
		$iframe.contents().find('head').prepend($('<head>'));
	}

	if($iframe.contents().find('head base').attr('target') != window.name) {
		$iframe.contents().find('head base').attr('target', window.name);
	}

	// Set height.
	var targetHeight = $(window).height();
	if($iframe[0].contentWindow.window != null && $iframe[0].contentWindow.window.document.body != null) {
		targetHeight = Math.max(
				targetHeight,
				$iframe[0].contentWindow.window.document.body.scrollHeight
		);
	}

	var actualHeight = $iframe.height()
	+ parseInt($iframe.contents().find('body').css('margin-top'))
	+ parseInt($iframe.contents().find('body').css('padding-top'))
	+ parseInt($iframe.contents().find('body').css('padding-bottom'))
	+ parseInt($iframe.contents().find('body').css('margin-bottom'));

	if(targetHeight > actualHeight || targetHeight < (actualHeight - 100)) {
		$iframe.height(targetHeight + 10);
	}

	// Set width.
	var targetWidth = $(window).width();
	if($iframe[0].contentWindow.window != null && $iframe[0].contentWindow.window.document.body != null) {
		targetWidth = Math.max(
				targetWidth,
				$iframe[0].contentWindow.window.document.body.scrollWidth
		);
	}

	var actualWidth = $iframe.width()
	+ parseInt($iframe.contents().find('body').css('margin-left'))
	+ parseInt($iframe.contents().find('body').css('padding-left'))
	+ parseInt($iframe.contents().find('body').css('padding-right'))
	+ parseInt($iframe.contents().find('body').css('margin-right'));

	if(targetWidth > actualWidth || targetWidth < (actualWidth - 100)) {
		$iframe.width(targetWidth + 10);
	}
}

if (!window.console) {
	console = {	log : function() {}	};
}

if (typeof String.prototype.startsWith != 'function') {
	String.prototype.startsWith = function (str){
		return this.lastIndexOf(str, 0) === 0;
	};
}

var base_url = "";
var relative_url = "";
for(var i in alt_base_urls) {
	if(window.location.href.startsWith(alt_base_urls[i])) {
		base_url = alt_base_urls[i];
		relative_url = window.location.href.substring(alt_base_urls[i].length);
		break;
	}
}

$('iframe').height($(window).height());

var ifrInt = setInterval(function() {
	if($('iframe').contents().find('body *:first').length > 0) {
		iframeLoaded($('iframe'));
		clearInterval(ifrInt);
	}
}, 300);

setTimeout(function() {
	if(!done) {
		console.log('emergency!');
		$('li a').each(function() {
			window.location = $(this).attr('href');
		});
	}
}, 20000);

$('iframe').load(function() {
	iframeLoaded($(this));
});

for(var i in alt_base_urls) {
	(function(alt_base_url) {
		if(window.location.href.startsWith(alt_base_url)) {
			return;
		}

		var alt_url = alt_base_url + relative_url;

		var alt_url_parser = document.createElement('a');
		alt_url_parser.href = alt_url;
		if(alt_url_parser.search) {
			alt_url += '&';
		} else {
			alt_url += '?';
		}
		alt_url += get_param_name + '=' + output_type_jsonp + encodeURIComponent(alt_base_url);

		$.ajax({
			dataType: 'jsonp',
			url: alt_url,
			cache: true,
			jsonpCallback: jsonp_callback_basename + i
		}).success(function(data) {
			if(!done && typeof data.html !== 'undefined') {
				done = true;
				console.log(alt_url + ' succeeded');
				$iframe = $('iframe').clone();
				$('iframe').remove();
				$iframe.attr('src', '');
				$('body').append($iframe);

				var iframeDoc = $iframe[0].contentDocument || $iframe[0].contentWindow.document;
				iframeDoc.open();
				iframeDoc.write(data.html);
				iframeDoc.close();

				iframeLoaded($iframe);
			}
		});
	})(alt_base_urls[i]);
}