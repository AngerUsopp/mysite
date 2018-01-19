<!DOCTYPE html>
<html>
<body>

<h1>汉字汉字汉字汉字</h1>

<?php
	echo "Hello World!";
	
	$arr = array(
		'name' => '汉字',
		'nick' => '汉字',
		'contact' => array(
			'email' => 'zhuoweida@163.com',
			'website' => 'http://zhuoweida.blog.tianya.cn',
		)
	);
	$json_string = json_encode($arr);
	echo "$json_string";
?>

</body>
</html>

