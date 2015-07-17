
<html>
<body>
Current time: <?php $ct = time(); echo "$ct"; ?><br>
The following files are present:<br>
<?php
foreach (glob("/var/tmp/btree_*") as $filename) {
    echo "$filename, size: " . filesize($filename) . ", modified: " . filemtime($filename) . "<br>\n";
}
foreach (glob("/var/tmp/AMI_*") as $filename) {
    echo "$filename, size: " . filesize($filename) . ", modified: " . filemtime($filename) . "<br>\n";
}
foreach (glob("/tmp/tplog*") as $filename) {
    echo "$filename, size: " . filesize($filename) . ", modified: " . filemtime($filename) . "<br>\n";
}

?>
To delete these files <a href="cleanup.php">click here</a>.
</body>
</html>
