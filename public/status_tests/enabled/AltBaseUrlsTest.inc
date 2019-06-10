<?php

class AltBaseUrlsTest extends StatusTest
{

    function run()
    {
        if (count(RedirectWhenBlockedFull::getAltBaseUrls()) == 0) {
            $this->messages[] = 'Empty alt base urls';
            return;
        }
        
        $domains = array();
        
        foreach (RedirectWhenBlockedFull::getAltBaseUrls() as $url) {
            
            // Test DNS poisoning.
            $domain = parse_url($url, PHP_URL_HOST);
            $domains[] = $domain;
        }
        
        if (! Conf::$china_ip_for_dns_poisoning_test) {
            $this->messages[] = 'china_ip_for_dns_poisoning_test not set';
            $this->n_failed ++;
        } else {
            $command = '';
            foreach ($domains as $domain) {
                $command .= '(dig +time=5 +tries=1 @' .
                     Conf::$china_ip_for_dns_poisoning_test . ' ' . $domain .
                     ' > /dev/null ; echo "' . $domain . ':"$?)  & ';
            }
            $command = trim($command) . '& wait';
            exec($command, $output);
            $n_domains_not_poisoned = 0;
            foreach ($output as $line) {
                $line_chunks = explode(':', $line, 2);
                if (count($line_chunks) == 2 && $line_chunks[0] &&
                     $line_chunks[1] >= 0) {
                    list ($domain, $dig_exit_code) = $line_chunks;
                    if ($dig_exit_code == 0) {
                        $this->messages[] = $domain . ' is poisoned (' .
                             $dig_exit_code . ')';
                    } else {
                        $this->messages[] = $domain . ' is not poisoned (' .
                             $dig_exit_code . ')';
                        $n_domains_not_poisoned++;
                    }
                } else {
                    $this->messages[] = 'unknown dig result (' . $line . ')';
                }
            }
            
            if(!$n_domains_not_poisoned) {
                $this->messages[] = 'no unpoisoned domains available';
                $this->n_failed ++;
            }
        }
        
        $alt_base_urls_file = dirname(dirname(__DIR__)) .
             '/rwb/conf/alt_base_urls.txt';
        $alt_base_urls_modified_ago = time() - filemtime($alt_base_urls_file);
        $this->messages[] = "$alt_base_urls_file modified $alt_base_urls_modified_ago seconds ago";
        if ($alt_base_urls_modified_ago > Conf::$alt_base_urls_modified_ago_max) {
            $this->messages[] = "$alt_base_urls_file expired";
            return false;
        }
        
        return true;
    }
}