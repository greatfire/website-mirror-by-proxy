<?php

abstract class StatusTest
{

    protected $messages = array();

    protected $n_failed = 0;

    protected $n_passed = 0;

    public function __construct()
    {
        if ($this->run()) {
            $this->n_passed ++;
        }
    }

    public function __toString()
    {
        $str = get_class($this) . ' ';
        $str .= $this->passed() ? 'passed' : 'did NOT pass';
        $str .= ":\n";
        $str .= print_r($this->getMessages(), true);
        return $str;
    }

    public function getMessages()
    {
        return $this->messages;
    }

    public function passed()
    {
        if ($this->n_passed > 0 && $this->n_failed == 0) {
            return true;
        }
    }

    protected abstract function run();
}