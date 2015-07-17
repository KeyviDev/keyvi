
<html>
<body>
Current time: <?php $ct = time(); echo "$ct"; ?><br>
The following files are being removed:<br>
<?php
foreach (glob("/var/tmp/btree_*") as $filename) {
    echo "$filename, size: " . filesize($filename) . ", modified: " . filemtime($filename) . "<br>\n";
    unlink($filename);
}
foreach (glob("/var/tmp/AMI_*") as $filename) {
    echo "$filename, size: " . filesize($filename) . ", modified: " . filemtime($filename) . "<br>\n";
    unlink($filename);
}

?>
</body>
</html>