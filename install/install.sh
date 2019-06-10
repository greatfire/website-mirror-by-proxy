#!/bin/bash
function pecl_install {
	pecl install $1
	echo "extension=$2.so" | sudo tee /etc/php5/mods-available/$2.ini
	echo "; priority=$3" | sudo tee -a /etc/php5/mods-available/$2.ini
	php5enmod $2
}

if [ $(/usr/bin/id -u) -ne 0 ]; then
    echo "Not running as root"
    exit
fi

echo "Installing Apache and PHP"
apt-get install apache2 php5 php5-curl

echo "Installing HTTP extension"
apt-get install libpcre3-dev libcurl3-openssl-dev php5-dev php-http php5-mcrypt php-pear
pecl_install pecl/raphf raphf 20
pecl_install pecl/propro propro 20
pecl_install pecl_http http 30

echo "Enabling Apache modules"
a2enmod proxy
a2enmod proxy_http
a2enmod rewrite
