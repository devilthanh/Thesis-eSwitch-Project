const char index_html[] PROGMEM = R"rawliteral(
	<!DOCTYPE html>

	<html lang="en">
		<head>
			<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
			<meta charset="UTF-8">
			<meta http-equiv="cache-control" content="no-cache">
			<meta http-equiv="expires" content="0">
			<meta http-equiv="pragma" content="no-cache">
			
			<title>eSwitch Config</title>
			<style>		
				* {
					box-sizing: border-box;
				}

				html {
					height:100%;
					margin: 0px;
					padding: 0px;
				}

				body {
					transition-duration: 0.5s;
					background: rgba(212,228,239,1);
					background: -moz-radial-gradient(center, ellipse cover, rgba(212,228,239,1) 0%, rgba(134,174,204,1) 100%);
					background: -webkit-gradient(radial, center center, 0px, center center, 100%, color-stop(0%, rgba(212,228,239,1)), color-stop(100%, rgba(134,174,204,1)));
					background: -webkit-radial-gradient(center, ellipse cover, rgba(212,228,239,1) 0%, rgba(134,174,204,1) 100%);
					background: -o-radial-gradient(center, ellipse cover, rgba(212,228,239,1) 0%, rgba(134,174,204,1) 100%);
					background: -ms-radial-gradient(center, ellipse cover, rgba(212,228,239,1) 0%, rgba(134,174,204,1) 100%);
					background: radial-gradient(ellipse at center, rgba(212,228,239,1) 0%, rgba(134,174,204,1) 100%);
					filter: progid:DXImageTransform.Microsoft.gradient( startColorstr='#d4e4ef', endColorstr='#86aecc', GradientType=1 );
					position: relative;
					width: 100%;
					height: 100%;
					margin: 0px;
					padding: 0px;
				}

				::placeholder {
				  color: #ccc;
				  opacity: 1; /* Firefox */
				}

				*:focus {
					outline: none;
				}
				
				@media (min-width: 699px) {
					.logincover {
						padding: 20px 30px;
						border-radius: 20px;
						background: rgba(255, 255, 255, .3);
						-moz-box-shadow: 0px 2px 5px 1px #444;
						-webkit-box-shadow: 0px 0px 15px -5px #444;
					}
				}
				
				.center {
					margin: 0;
					position: absolute;
					top: 50%;
					left: 50%;
					-ms-transform: translate(-50%, -50%);
					transform: translate(-50%, -50%);
				}

				.mainbutton {
					box-shadow: inset 0px 1px 0px 0px #7a8eb9;
					background: linear-gradient(to bottom, #637aad 5%, #5972a7 100%);
					background-color: #637aad;
					border: none;
					display: inline-block;
					cursor: pointer;
					color: #ffffff;
					font-family: Arial;
					font-size: 16px;
					font-weight: bold;
					padding: 6px 12px;
					text-decoration: none;
					height: 40px;
					border-radius: 30px;
					width: 150px;
					margin-top: 20px
				}
				.mainbutton:hover {
					background:linear-gradient(to bottom, #5972a7 5%, #637aad 100%);
					background-color: #5972a7;
				}
				.mainbutton:active {
					position:relative;
					top:1px;
				}

				.maininput {
					 padding: 10px;
					 border-width: 1px;
					 border-color: #CCCCCC;
					 background-color: #FFFFFF;
					 color: #637aad;
					 border-style: solid;
					 border-radius: 5px;
					 box-shadow: 0px 0px 5px rgba(66,66,66,.75);
					 font-weight: bold;
					 width: 250px;
					 margin-top: 0px;
					 font-size: 15px;
					 margin-bottom: 10px;
				}

				.maininput:focus {
					 outline:none;
				}

				.maintitle {
					font-size: 25px;
					color: #637aad;
					font-weight: bold;
					padding-left: 0px;
					margin-bottom: 5px;
					margin-top: 15px;
					font-family: Arial;
				}

				.mainlabel {
					color: #637aad;
					font-size: 15px;
					padding-left: 0px;
					margin-bottom: 5px;
					font-weight: bold;
				}
				
			</style>
		</head>
		<body id="body" style="transition-duration: 0.5s;background-size:cover;background-color:#151515" onload="onLoad()">
			<div class="center logincover" align="center">
				<b><p align="center" class="maintitle">eSwitch Config</p></b>
				<p id="status" align="center" style="margin:0px;margin-bottom:10px;font-size:17px;color:#637aad ;height:20px"></p>
				<form align="center" style="color:#ffffff;font-size:17px;width:250px" onsubmit="onSubmit()">
					<b><p align="left" class="mainlabel">Wifi SSID:</p></b>
					<input id="wifiSSID" name="wifiSSID" placeholder="Your Wifi SSID" class="maininput" align="left" type="text">
					<b><p align="left" class="mainlabel">Wifi Password:</p></b>
					<input id="wifiPassword" name="wifiPassword" placeholder="Your Password" class="maininput" align="left" type="text">
					<b><p align="left" class="mainlabel">User ID:</p></b>
					<input id="userID" name="userID" placeholder="Your User ID" class="maininput" align="left" type="text">
					<input id="submit" name="submit" class="mainbutton" type="submit" value="Save">
					<input name="status" type="hidden" value="none">
				</form>
			</div>
		</body>
		<script>
			onLoad = function(){
				var header = window.location.search.substring(1);
				var params = header.split("&");
				for (i = 0; i < params.length; i++) {
					var param = params[i].split("=");
					document.getElementById(param[0]).value = unescape(param[1]);
					if(param[0] == "status" && param[1] == "saved"){
						document.getElementById(param[0]).innerHTML = "Save successfully! Restarting...";
					}
				}
			}
			
			onSubmit = function(){
				document.getElementById("status").innerHTML = "";
			}
		</script>
	</html>

)rawliteral";

const char redirect_html[] PROGMEM = R"rawliteral(	
	<!DOCTYPE html>
	<html>
		<head>
			<title>HTML Meta Tag</title>
			<meta http-equiv = "refresh" content = "0; url=http://8.8.8.8/" />
			<script type="text/javascript">
				window.location.href = "http://8.8.8.8/";
			</script>
			<title>Redirecting...</title>
		</head>
		<body>
		  <p>Redirecting...</p>
		</body>
	</html>
)rawliteral";