<html>
  <head>
    <title>B+-tree</title>
    <link rel="stylesheet" title="(Default)" href="dukecs.css" type="text/css" media="screen"> 
    <style>
      .node0 { background-color: #557; color: #00b; font-size: normal; }
      .node1 { background-color: #668; color: #00b; font-size: normal; }
      .node2 { background-color: #779; color: #00b; font-size: normal; }
      .node3 { background-color: #88a; color: #00b; font-size: normal; }
      .node4 { background-color: #99b; color: #00b; font-size: normal; }
      .node5 { background-color: #aac; color: #00b; font-size: normal; }
      .node6 { background-color: #bbd; color: #00b; font-size: normal; }
      .node7 { background-color: #cce; color: #00b; font-size: normal; }
      .node8 { background-color: #ddf; color: #00b; font-size: normal; }

      .leaf { background-color: #ddf; color: #800; font-size: normal; }

      .key0 { background-color: #557; color: #11b; text-align: center;  font-size: normal; }
      .key1 { background-color: #668; color: #11b; text-align: center;  font-size: normal; }
      .key2 { background-color: #779; color: #11b; text-align: center;  font-size: normal; }
      .key3 { background-color: #88a; color: #11b; text-align: center;  font-size: normal; }
      .key4 { background-color: #99b; color: #11b; text-align: center;  font-size: normal; }
      .key5 { background-color: #aac; color: #11b; text-align: center;  font-size: normal; }
      .key6 { background-color: #bbd; color: #11b; text-align: center;  font-size: normal; }
      .key7 { background-color: #cce; color: #11b; text-align: center;  font-size: normal; }
      .key8 { background-color: #ddf; color: #11b; text-align: center;  font-size: normal; }

      div#param { background-color: #ddf; padding: 6px; margin: 0px;
                  position: absolute; right: 8px; top: 8px; width: 18%; font-size: normal; }
      .bluebg   { background-color: #ddf; }
      div#intro { position: relative; width: 80%; margin: 9px 0px 0px 0px; font-size: normal;  }
    </style>
  </head>

  <body>
    
    <h1>B<sup>+</sup>-tree Visualization</h1>
    <div id=menu><a href="index.php">Build another B+-tree</a> | 
    <a href="http://www.cs.duke.edu/~tpie/">TPIE home page</a> | 
    <a href="http://www.cs.duke.edu/~tavi/">Tavi's home page</a></div>

    <div id=intro>
    <table cellspacing=6px cellpadding=6px>
    <tr> <td class=bluebg width=75%>
    <strong>Explanation</strong><br>The B<sup>+</sup>-tree is represented
    "sideways," in tabular form: The root is in the leftmost column and the
    leaves are in the rightmost column. <strong>A shaded area represents a node</strong>,
    and all nodes on the same level use the same shade. The values shown
    inside an internal node are discriminator <span class=key8>keys</span>
    and <span class=node8>[ID]</span>s. Where appropriate, the left bracket
    of an ID links to the previous sibling, and the right bracket links to
    the next sibling. The values shown inside a leaf node are (the keys of)
    <span class=leaf>the records</span> stored in that leaf. Leaf node IDs
    are not shown. <br><small><strong>NB:</strong> This representation is
    meant only for understanding the structure of the B+-tree; in an
    implementation, this may not be the most efficient way of storing nodes
    (in terms of space and cache efficiency).</small></td>
    <td class=bluebg valign=top>
      <strong>Insertions</strong><br>
      <form method="post" action="btree.php">
<?php 
  $sneaky = "";
  $btn = $_POST["btn"]; 
  if (!is_numeric($btn)) $btn = 9;
  $lns = $_POST["lns"];
  if (!is_numeric($lns)) {
    $sneaky = "!!![lns][$lns]";
    $lns = 40;
  }
  $fan = $_POST["fan"];
  if (!is_numeric($fan)) {
    $sneaky = "!!![fan][$fan]";
    $fan = 10;
  }
  $blm = $_POST["blm"];
  echo "<input type=\"hidden\" name=\"lns\" value=\"$lns\">\n"; 
  echo "<input type=\"hidden\" name=\"fan\" value=\"$fan\">\n"; 
  echo "<input type=\"hidden\" name=\"blm\" value=\"$blm\">\n"; 
  $ref = $_SERVER["HTTP_REFERER"];
  if ($ref != "http://www.cs.duke.edu/~tavi/btree/btree.php") {
    $btn = rand(10,25);
    // do some cleanup 
    while (file_exists("/var/tmp/btree_$btn")) { 
      if (time() - filemtime("/var/tmp/btree_$btn") > 1200) { 
        break;  
      } else { 
        $btn = rand(10,25); 
      } 
    } 
    touch("/var/tmp/btree_$btn");
  }
 
  echo  "<input type=\"hidden\" name=\"btn\" value=\"$btn\">\n";
  echo "    
	Insert new values into this B+-tree (space-separated):
	<p><input name=\"ins\" size=15 maxlen=256> 
	<input type=\"submit\" value=\"Insert\"> </p>
	Newly inserted values appear in <span class=leaf><b>bold</b></span>
	face. 
	</form> 
	</td>    
	</tr>   
	</table>  
      </div>   
      <p>
        ";


     if ($ref != "http://www.cs.duke.edu/~tavi/btree/" && 
         $ref != "http://www.cs.duke.edu/~tavi/btree/index.php" && 
	 $ref != "http://www.cs.duke.edu/~tavi/btree/btree.php") {
       exit("<strong>An error has occured. Please <a href=\"index.php\">try again</a>.</strong>");
     } 
     $ins = escapeshellarg($_POST["ins"]);

     # Check whether to insert into existing btree.
     if ($ref == "http://www.cs.duke.edu/~tavi/btree/btree.php") {
       
       clearstatcache();

       if (!file_exists("/var/tmp/btree_$btn") || (time() - filemtime("/var/tmp/btree_$btn") > 1197)) {
         exit("<strong>The B+-tree has expired or an error has occured. Please <a href=\"index.php\">build a new B+-tree</a>.</strong>");
       } else {
         touch("/var/tmp/btree_$btn");
       }
       $insarg = (empty($ins)) ? " " : " --insert $ins ";
       $blmarg = ($blm == "i") ? " " : " -b ";
       if (file_exists("/var/tmp/btree_$btn.l.blk")) {
         system("./visualize_btree -l $lns -n $fan --btree-name /var/tmp/btree_$btn $insarg -H $blmarg");
       } else {
         exit("<strong>The B+-tree has expired or an error has occured. Please <a href=\"index.php\">build a new B+-tree</a>.</strong>");
       }

       $file = "/var/tmp/btree_$btn";
       $file_sz = 0;

     } else {

       if ($_FILES["ufile"]["name"]) {
	 $orig_file = $_FILES['ufile']['name']; 
         $file = "/var/tmp/btree_$btn.input";
	 clearstatcache();
         #if (!file_exists($file) || ($_FILES['ufile']['error'] != UPLOAD_ERR_OK)) 
	 if (!move_uploaded_file($_FILES['ufile']['tmp_name'], $file)) {
	   exit("<strong>An error has occured while uploading file \"$orig_file\". Please <a href=\"index.php\">try again</a>.</strong>");
	 }
         $uploaded = true;
         #echo "Input file used:  $orig_file (uploaded)<br>\n";
	 #echo "Server file:  $file (uploaded)<br>\n";
       } else {
         $file = $_POST["sfile"];
         $uploaded = false;
       }

       if (file_exists("/var/tmp/btree_$btn.l.blk"))
         unlink("/var/tmp/btree_$btn.l.blk");
       if (file_exists("/var/tmp/btree_$btn.n.blk"))
         unlink("/var/tmp/btree_$btn.n.blk");

       # insert or bulk load?      
       $blmarg = ($blm == "i") ? " " : " -b ";
       system("./visualize_btree -i $file -l $lns -n $fan -H $blmarg --btree-name /var/tmp/btree_$btn");

       $file_sz = file_exists($file) ? filesize($file): "err";
              
       if ($uploaded && file_exists($file)) unlink($file);
     }

     # write a log entry.
 
     $ua = $_SERVER["HTTP_USER_AGENT"];
     $ra = $_SERVER["REMOTE_ADDR"];
     $dt = date("Y-m-d H:i:s");
     $handle = fopen("/usr/xtmp/tavi/btree_btree.log", "a");
     if ($handle) {
       fwrite($handle, "\"$dt\",\"$ua\",\"$ra\",\"$ref\",\"$file\",\"$file_sz\"");
       if ($sneaky)
         fwrite($handle, ",\"$sneaky\"\n");
       else
         fwrite($handle, "\n");
       fclose($handle);
     }
   
    ?>
    <hr> Generated on <?php echo date("D M j  G:i:s T Y"); ?>. 
    Maintained by <a href="mailto:tavi@cs.duke.edu">tavi@cs.duke.edu</a>.


  </body>
</html>
