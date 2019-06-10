<?php

class DomUrlFilters
{

    private static $attributes = array();

    public static function addAttribute($attribute, $element = '*')
    {
        self::$attributes[$attribute] = $element;
    }

    public static function applyAll(simple_html_dom $dom, 
        ProxyHttpRequest $request)
    {
        foreach (self::$attributes as $attribute => $elements) {
            self::apply($dom, $attribute, $elements, $request);
        }
    }

    private static function apply(simple_html_dom $dom, $attribute, $elements, 
        ProxyHttpRequest $request)
    {
        foreach ($dom->find($elements . '[' . $attribute . ']') as $element) {
            $attr_val = $element->getAttribute($attribute);
            if ($attr_val) {
                
                // Ignore these links.
                if (startsWith($attr_val, array(
                    'data:',
                    'javascript:',
                    'mailto:',
                    '.'
                ))) {
                    continue;
                }
                
                // Turn protocol relative URLs into HTTPS.
                if (isset($attr_val[1]) && $attr_val[1] == '/' &&
                     $attr_val[0] == '/') {
                    $attr_val = 'https:' . $attr_val;
                    $element->setAttribute($attribute, $attr_val);
                } 

                else {
                    $attr_val_components = parse_url($attr_val);
                    
                    // Nothing to do without paths.
                    if (isset($attr_val_components['path'])) {
                        
                        // Remove current host.
                        if (isset($attr_val_components['host']) && $attr_val_components['host'] ==
                             $request->getUrlComponent('host')) {
                            unset($attr_val_components['host']);
                        }
                        
                        // If URL without host.
                        if (! isset($attr_val_components['host'])) {
                            
                            // If path does not start with a slash, prepend current path directory.
                            if ($attr_val_components['path'][0] != '/') {
                                $attr_val_components['path'] = dirname(
                                    $request->getUrlComponent('path')) . '/' .
                                     $attr_val_components['path'];
                            }
                            
                            $attr_val = '.' . http_build_path_query_fragment(
                                $attr_val_components);
                            $element->setAttribute($attribute, $attr_val);
                        }
                    }
                }
                
                /*
                 * Special for existing base href values. They should always end with a slash,
                 * but browsers are lenient if it's eg http://example.com. But since we may rewrite
                 * the value later, it might have a path and without the slash it will fail.
                 */
                if ($element->tag == 'base') {
                    $last_char = $attr_val[strlen($attr_val) - 1];
                    if ($last_char != '/') {
                        $attr_val .= '/';
                        $element->setAttribute($attribute, $attr_val);
                    }
                }
            }
        }
    }
}