<?php
	//echo "Hello World!";
	
	//$arr = array(
	//	'name' => '魏艳辉',
	//	'nick' => '为梦翱翔',
	//	'contact' => array(
	//		'email' => 'zhuoweida@163.com',
	//		'website' => 'http://zhuoweida.blog.tianya.cn',
	//	)
	//);
	//$json_string = json_encode($arr);
	//echo "$json_string";
	
	echo "_GET -> " . $_GET["p0"] . "<br>";
	echo "_GET -> " . $_GET["p1"] . "<br>";
	
	if ($_SERVER["REQUEST_METHOD"] == "POST") 
	{
		$post_data = file_get_contents('php://input');
		// always_populate_raw_post_data=On
		//$post_data = $GLOBALS['HTTP_RAW_POST_DATA'];
		echo "raw post data<br>";
		echo $post_data . "<br>";
		
		echo "json_decode data<br>";
		$josn_data = json_decode(trim($post_data), true);
		var_dump($josn_data) . "<br>";
		
		echo $josn_data["name"] . "<br>";
		echo $josn_data["nick"] . "<br>";
	}
	
	echo "REQUEST_METHOD -> " . $_SERVER['REQUEST_METHOD'] . "<br>";
	echo "QUERY_STRING -> " . $_SERVER['QUERY_STRING'] . "<br>";
	echo "SERVER_NAME -> " . $_SERVER['SERVER_NAME'] . "<br>";
	echo "REMOTE_PORT -> " . $_SERVER['REMOTE_PORT'] . "<br>";
	echo "PHP_SELF -> " . $_SERVER['PHP_SELF'] . "<br>";
	echo "argv -> " . $_SERVER['argv'] . "<br>";
	echo "argc -> " . $_SERVER['argc'] . "<br>";
	echo "SERVER_SOFTWARE -> " . $_SERVER['SERVER_SOFTWARE'] . "<br>";
	echo "SERVER_PROTOCOL -> " . $_SERVER['SERVER_PROTOCOL'] . "<br>";
	echo "HTTP_ACCEPT -> " . $_SERVER['HTTP_ACCEPT'] . "<br>";
	echo "HTTP_ACCEPT_CHARSET -> " . $_SERVER['HTTP_ACCEPT_CHARSET'] . "<br>";
	echo "HTTP_ACCEPT_ENCODING -> " . $_SERVER['HTTP_ACCEPT_ENCODING'] . "<br>";
	echo "HTTP_ACCEPT_LANGUAGE -> " . $_SERVER['HTTP_ACCEPT_LANGUAGE'] . "<br>";
	echo "HTTP_USER_AGENT -> " . $_SERVER['HTTP_USER_AGENT'] . "<br>";
	echo "SERVER_PORT -> " . $_SERVER['SERVER_PORT'] . "<br>";
	echo "SCRIPT_NAME -> " . $_SERVER['SCRIPT_NAME'] . "<br>";
	echo "HTTP_HOST -> " . $_SERVER['HTTP_HOST'] . "<br>";
	echo "REMOTE_ADDR -> " . $_SERVER['REMOTE_ADDR'] . "<br>";
	echo "CONTENT_TYPE -> " . $_SERVER['CONTENT_TYPE'] . "<br>";
	echo "CONTENT_LENGTH -> " . $_SERVER['CONTENT_LENGTH'] . "<br>";
	
	
	//$p0 = "p0";
	//$p1 = "p1";	
	//function test_fn()
	//{
	//	//echo $GLOBALS['p0'] . "<br>";
	//	
	//	//global $p1;		
	//	//echo $p1;
	//	
	//	static $p2 = 22;
	//	$p2++;
	//	echo $p2 . "<br>";
	//}
	//for($i = 0; $i < 2; $i++)
	//{
	//	test_fn();
	//}
	
	//$vct = array("f", "s", "t");
	//foreach($vct as $item)
	//{
	//	echo $item . "<br>";
	//}
	
	//abstract class IInterface
	//{
	//	function __construct($name)
	//	{
	//		$this->name = $name;
	//		
	//		static $id = 0;
	//		$this->id_ = ++$id;
	//		
	//		echo $this->id_ . " IInterface::__construct<br>";
	//		
	//		register_shutdown_function(array($this, "before_destruct"));			
	//	}
	//	
	//	function __destruct()
	//	{
	//		echo $this->id_ . " IInterface::__destruct<br>";
	//	}
	//	
	//	function myname()
	//	{
	//		echo $this->name . "<br>";
	//	}
	//	
	//	function myid()
	//	{
	//		echo "id = " . $this->id_ . "<br>";
	//	}
	//	
	//	function before_destruct()
	//	{
	//		echo $this->id_ . " IInterface::before_destruct<br>";
	//	}
	//	
	//	var $name;
	//	var $id_;
	//}
	//class CImpl extends IInterface
	//{
	//	function __construct($name = "no_name")
	//	{
	//		parent::__construct($name);
	//	}
	//	
	//	function __destruct()
	//	{			
	//		echo $this->id_ . " CImpl::__destruct<br>";
	//		
	//		parent::__destruct();
	//	}		
	//}
	//for($i = 0; $i < 2; $i++)
	//{
	//	$impl = new CImpl();
	//	$impl->myid();
	//}
	
	//$cars = array("Volvo", 121, 3.14, 5.96e2, 0xf1, 07);
	//echo var_dump($cars) . "<br>";
	//$x = 1;
	//var_dump($x);
?>