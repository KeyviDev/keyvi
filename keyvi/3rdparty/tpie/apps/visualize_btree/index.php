<html>
  <head>
    <title>Build a B+-tree</title>
    <link rel="stylesheet" title="(Default)" href="dukecs.css"
      type="text/css" media="screen">
  </head>

<?php 
  $ua = $_SERVER["HTTP_USER_AGENT"]; 
  $ref = $_SERVER["HTTP_REFERER"]; 
  $ra = $_SERVER["REMOTE_ADDR"]; 
  $dt = date("Y-m-d H:i:s");
  $handle = fopen("/usr/xtmp/tavi/btree_index.log", "a"); 
  if ($handle) { 
    fwrite($handle, "\"$dt\",\"$ua\",\"$ra\",\"$ref\"\n"); 
    fclose($handle); 
  } 
?> 

  <body>
    <h1>Build a B<sup>+</sup>-tree</h1>
    <div id=menu><a href="http://www.cs.duke.edu/~tpie/">TPIE Home Page</a>
      | <a href="http://www.cs.duke.edu/~tavi/">Tavi's Home Page</a></div>
    <p>The <strong>B-tree</strong> is the classic disk-based data structure
      for indexing records based on
      an ordered key set. The <strong>B<sup>+</sup>-tree</strong>
      (sometimes written B+-tree, B+tree, or just B-tree) is a variant of the
      original B-tree
      in which all records are stored in the leaves and all leaves are
      linked sequentially. The
      B+-tree is used as a (dynamic) indexing
      method in relational database management systems. See the
      bibliography below for more on B-trees, B+-trees, and
      other variants.
    </p>

    <p>Use this form to <strong>build</strong> and <strong>visualize</strong> a  B+-tree on
      integer keys. You can upload an input file or
      use a sample file from the menu.<br>The visualization method is scalable and lightweight; it uses only HTML and CSS -- no plugin is necessary.</p>

    <div style="background-color: #ddf; width: 99%; padding: 4px;">
    <form enctype="multipart/form-data" method="post" action="btree.php">
      <input type="hidden" name="MAX_FILE_SIZE" value="256000">
      <table cellpadding=0 border=0 cellspacing=3>
	<tr><td align=right>Upload file containing space-separated
	      integers: </td><td><input size=25
	      name="ufile" type="file"></td></tr>
	<tr><td align=right>or Choose a sample file (if no file uploaded): </td><td>
	    <select name="sfile" 
		title="Each file contains randomly generated integers between 0 and 999,999">
	      <option value="r0100"> r0100 (100 random integers) </option>
	      <option value="r0500"> r0500 (500 random integers) </option>
	      <option value="r1000" selected> r1000 (1000 random integers) </option>
	      <option value="r5000"> r5000 (5000 random integers) </option>
	    </select></td>
	</tr>
	<tr><td align=right>Maximum leaf size: </td><td><input 
		title="Between 2 and 2045 (each leaf node is stored in one disk block)"
		name="lns" size=4 value="40" maxlen=4> records</td></tr>
	<tr><td align=right>Maximum fanout: </td><td><input 
		title="Between 4 and 1024 (each internal node is stored in one disk block)"
		name="fan" size=4 value="10" maxlen=4></td></tr>
	<tr><td align=right>Building method:</td><td><input type="radio" checked name="blm"
	      value="i">Repeated insertion <input type="radio" name="blm"
	      value="b">Bulk load</td></tr>
	<tr><td align=center colspan=2><input type="submit" value="Build B+-tree" style="width:14em"></td></tr>
      </table>
    </form>
    </div>
    <p>The B<sup>+</sup>-tree implementation code is written in C++ and is part of the
      free <a href="http://www.cs.duke.edu/~tpie/">TPIE</a> libray.</p>
    <h4>Bibliography</h4>
    <ol>
      <li>Rudolf Bayer, Edward M. McCreight: Organization and Maintenance
	of Large Ordered Indices. Acta Informatica 1:
	173-189(1972). <em>The original B-tree paper.</em></li>
      <li>Douglas Comer: The Ubiquitous B-Tree. ACM Computing Surveys
	11(2): 121-137(1979). <em>An excellent survey on the B-tree and its
      variants.</em></li>
      <li><em><a
	  href="http://www.informatik.uni-trier.de/~ley/db/access/btree.html">An 
	    extensive list of research papers</a> on B-trees, B+-trees, and
	  other variants, from 
	  <a href="http://www.informatik.uni-trier.de/~ley/db/">dblp</a>.</em></li>
      <li>David Lomet. <a
	  href="http://www.cs.duke.edu/~junyang/cps216/papers/lomet-2001.pdf">The 
	  Evolution of Effective B-tree Page Organization and Techniques: 
	  A Personal Account</a>. SIGMOD Record, 2001. <em>Discusses the 
	  finer points of B+-tree page organization with respect to space 
	  and cache efficiency.</em></li>
      <li>Lars Arge, Octavian Procopiuc, Jeffrey S. Vitter. <a
	  href="http://www.cs.duke.edu/~tavi/papers/tpie_paper.pdf">Implementing 
	  I/O-Efficient Data Structures Using TPIE</a>. 
          Proc. 10th European Symposium on Algorithms (ESA '02),
          September 2002. <em>A recent paper
	  describing the <a href="http://www.cs.duke.edu/~tpie/">TPIE</a> library, which contains the B+-tree
	  implementation used on this page.</em></li>
      <li>Rick Grehan. <a
	  href="http://www.fawcette.com/javapro/2002_01/magazine/features/rgrehan/">How 
	  to Climb a B-tree</a>. JAVAPro Magazine, January 2002. 
	<em>An extensive magazine article detailing a Java implementation
	  of the B-tree.</em></li>
      <li>Ian G. Graham. <a
	  href="http://www.onthenet.com.au/~grahamis/int2008/week10/lect10.html#multi">Data 
	  Structures and Algorithms: B-Tree</a>. <em>Lecture notes (from a data structures
	  course) explaining 
	  the structure of a B+-tree, as well as the insertion and deletion algorithms.</em></li>
      <li>Jun Yang. <a
	  href="http://www.cs.duke.edu/~junyang/cps216/lectures/09-index.pdf">Advanced 
	  Database Systems: Indexing</a>. <em>Lecture notes (from a
	  databases course) explaining the structure of the B+-tree, 
	  insertion, deletion, bulk loading, and database-specific concerns.</em></li>
    </ol>

    <hr>
<!-- hhmts start -->
Last modified: Tue Jul 15 17:27:02 EDT 2003
<!-- hhmts end -->
    by <a href="mailto:tavi@cs.duke.edu?subject=B+-tree%20web%20page">tavi@cs.duke.edu</a>.
  </body>
</html>
