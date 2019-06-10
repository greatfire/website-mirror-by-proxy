## Introduction

Website-mirror-by-proxy is a server-side web proxy designed to host one or multiple dynamic mirror versions of any website. It is based on https://github.com/greatfire/redirect-when-blocked (the full edition). Whereas redirect-when-blocked requires the source/origin website to be modified, website-mirror-by-proxy runs separately and does not need any modification of the source/origin website.

The Wall Street Journal beautifully illustrated our approach with one inaccuracy. The person inside the country experiencing online censorship does not need any special software to access ban sites. Any regular browser will be able to access the dynamic mirrors. 

![alt tag](http://si.wsj.net/public/resources/images/BT-AA566A_CFIRE_9U_20150316165711.jpg)

## Preparation

You need to have accounts on CDNs or servers/VPS. You can set up accounts on [most CDNs])(http://www.cdnplanet.com/cdns/) and hosting providers. 

## How to set up
1. Install the required dependencies. If you are using Ubuntu or a similar OS you can use the install.sh script. Otherwise, manually install/enable Apache, the Apache rewrite module, PHP and the PHP HTTP extension (http://php.net/manual/en/book.http.php). It has to be version 1 of the HTTP extension since version 2 is not backward compatible. The specific version used in the install script and which this project has been tested successfully on is pecl_http-1.7.6.
2. Copy conf-local-template.inc to conf-local.inc.
3. In the conf-local.inc file, add the Conf::$default_upstream_base_url that you want to proxy. The URL should be formatted like this: scheme://domain, without any trailing slash or path. Example: http://example.com.
4. Make sure that Apache is parsing .htaccess files in the site directory (eg "AllowOverride All").
5. Add a list of one or more URLs where the site can be accessed to public/rwb/conf/alt_base_urls.txt. Each URL should include a trailing slash (eg http://localhost/website-mirror-by-proxy/public/ or http://example.com/).
6. Optionally add a list of one or more third-party URLs, where the user should be redirected to if all of the mirror URLs fail, to public/rwb/conf/alt_url_collections.txt.
7. Access the site...

There are many configuration settings, some of which are used by default in the 'main.inc' file. The static classes in the filters directory can be used to fix broken URL rewrites (usually because of URLs generated in javascript) and to proxy URLs on third-party hosts.

## Other platforms
This project could run on older versions of PHP, without the HTTP Extension, by replacing the HttpRequest, HTTPMessage etc with other compatible classes. It could run on servers other than Apache by adapting the .htaccess/rewrite functionality. It could also be wholly ported to a non-PHP environemnt, though such an initiative should probably start by porting redirect-when-blocked itself first.

## Live examples and screenshots

https://boxun1.azurewebsites.net/ 

![alt tag](https://lh3.googleusercontent.com/jxAZsTnTex0gW3ItSuP4yOnPpvEl5CFUQdH-T0nl-oun7mrzSy_lz5TbA2I-5ATVLYBdbYS-ak4uriPL9gwzoMm5bz09LuLKzFF1K20qbek4tOccZcXDIw0jLAUwu8m3cS8axWs)

## FAQ

Does the mirror update in real time and support editing?

Yes. The mirror site is a proxy of the original site. All changes on the original site will be effective immediately on the mirror sites. Furthermore, users can directly interact with the mirror sites and all changes/forms submitted by users will be immediately uploaded to the original site. 

We will questions and answers to the FAQ here when we receive them. Please submit issues on: https://github.com/greatfire/website-mirror-by-proxy/issues 

## Security Audit 

Independent security experts have audited the source code. The audit report will be made available once issues detailed in the report are fixed.  



## Contributions
.. are very welcome, as is feedback. Feel free to open issues and to contribute improvements.
