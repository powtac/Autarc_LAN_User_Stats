<!DOCTYPE html>
<html style="min-height:100%; position:relative">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<title>API Test</title>
	<script src="//code.jquery.com/jquery-1.10.2.js"></script>
	<script type="text/javascript">
	$(document).ready(function() {
		$("#tests").submit(function( event ) {
			event.preventDefault();

			var posting = $.post($(this).find("#url").val(), $(this).find("#body").val());
			
			posting.always(function(){
				$("#result, #code").empty();
				
				// Reload iframe
				$("#iframe").attr("src", function ( i, val ) { return val; });
				
			}).done(function(data, a, b) {
				$("#result").append( data );
				$("#code").append( b.status );
				
			}).fail(function(data) {
				$("#code").append( data.status );
			});
		});
		
		$("#iframe").load( function() {
			var top = 0;
			do {
				top = $("#iframe").contents().scrollTop();
				$("#iframe").contents().scrollTop( $("#iframe").contents().scrollTop() + 1000 );
			} while($("#iframe").contents().scrollTop() != top)
		});
	});
	
	</script>
</head>
<body style="height:100%">
	<div id="left" style="float: left; width:50%; display:inline-block;">
		<form action="test_posts.php" method="post" id="tests" arget="_blank">
			<input type="text" id="url" value="./../api/store/mac"/><br />
			<textarea id="body" rows="10" cols="50" wrap="off">{}</textarea><br />
			<input type="submit" name="submit"/>
		</form>
		<br />
		<div style="white-space:pre; font-family:monospace" id="code"></div><br />
		<div style="white-space:pre; font-family:monospace" id="result"></div>
	</div>
	<div id="right" style="display:inline-block; height:100%; left:50%; bottom:0; overflow:hidden; position:absolute; width:50%;">
		<iframe id="iframe" src="./../tmp/requests.txt" style="width:100%; height:100%; border:0 none;"/>
	</div>
</body>
</html>
