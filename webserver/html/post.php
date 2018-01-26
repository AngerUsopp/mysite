<?php

	// get方式提取参数 --------------------------------------------------------------
	//echo "_GET -> " . $_GET["p0"] . "<br>";
	//echo "_GET -> " . $_GET["p1"] . "<br>";
	
	// post方式提取数据 --------------------------------------------------------------
	if ($_SERVER["REQUEST_METHOD"] == "POST") 
	{
		// enctype="multipart/form-data" 的时候php://input 是无效的
		$post_data = file_get_contents('php://input');
		echo "raw post data<br>";
		echo $post_data . "<br>";
		
		echo "_POST<br>";
		var_dump($_POST);
		
		echo "_FILES<br>";
		var_dump($_FILES);
		
		echo "_REQUEST<br>";
		var_dump($_REQUEST);
	}
	
	echo "<br><br>  " . time() . "  client request details --------------------------------------------------------------<br><br>";
	var_dump($_SERVER);
?>