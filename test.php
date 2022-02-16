<?php
    $selfURL =  "http://" . $_SERVER['SERVER_NAME'] . $_SERVER['SCRIPT_NAME'];//URL pointing to this script (for automatic recursive calls)

    if (isset($_GET["file"])) {
        $reqFile = $_GET["file"];//Path of the requsted file (relative within DereGlobus project)
    }else {
        $reqFile = "DereGlobus/DereGlobus.kml";//Default: Top level file of DereGlobus
    }
    
    // Creates the KML/XML Document.
    $dom = new DOMDocument('1.0', 'UTF-8');
    $dom->load($reqFile);
    
    $root = $dom->documentElement;
    processNode($dom, $root, $selfURL);
    
    //For test puropses: appends a node with the extracted self URL
    //$testNode = $dom->createElement('TestSelfUrl', $selfURL);
    //$root->appendChild($testNode);
    
    // Creates the root KML element and appends it to the root document.
    //$node = $dom->createElementNS('http://earth.google.com/kml/2.1', 'kml');
    //$parNode = $dom->appendChild($node);
    
    // Creates a KML Document element and append it to the KML element.
    //$dnode = $dom->createElement('Document');
    //$docNode = $parNode->appendChild($dnode);
    
    $kmlOutput = $dom->saveXML();
    //header('Content-type: application/vnd.google-earth.kml+xml');
    echo $kmlOutput;
    
    function processNode (&$dom, &$node, &$selfURL) {
        //TODO: Maybe it is possible to skip some nodes whicha re not relevant
        foreach ($node->childNodes as $childNode) {
            if($childNode->nodeName == "NetworkLink"){
                replaceNetworkLinkContent($dom, $childNode, $selfURL);
                
                //For test purposes only: Mark each NetworkLink which has been found
                //$testNode = $dom->createElement('FoundNetworkLink', 'FoundNetworkLink');
                //$childNode->appendChild($testNode);
            }else{
                processNode($dom, $childNode, $selfURL);//Recursive call
            }
        }
    }
    
    function replaceNetworkLinkContent(&$dom, &$networkLinkNode, &$selfURL) {
        //Collect Information
        foreach ($networkLinkNode->childNodes as $childNode) {  
            if ($childNode->nodeName == "ExtendedData") {
                //Search for extended data relvant for the replacement
                foreach ($childNode->childNodes as $extendedDataChildNode) {
                    if($extendedDataChildNode->nodeName == "dg:FilePath"){
                        $filePath = $extendedDataChildNode->nodeValue;
                    }
                }
                
                //For test purposes only: Mark each ExtendedData which has been found
                //$testNode = $dom->createElement('FoundExtendedData', 'FoundExtendedData');
                //$childNode->appendChild($testNode);
            }elseif ($childNode->nodeName == "Link") {
                //Grab Link node and important children
                $linkNode = $childNode;
                foreach ($linkNode->childNodes as $linkChildNode){
                    if($linkChildNode->nodeName == "href") {
                        $linkNode_hrefNode = $linkChildNode;
                    }elseif ($linkChildNode->nodeName == "httpQuery") {
                        $linkNode_queryNode = $linkChildNode;
                    }
                }
                
                //For test purposes only: Mark each ExtendedData which has been found
                //$testNode = $dom->createElement('FoundLink', 'FoundLink');
                //$childNode->appendChild($testNode);
            }
        }
        
        //Perform replacements dependent on the found extended data
        if(isset($filePath)){
            $linkNode_hrefNode->nodeValue = $selfURL . "?" . "file=" . $filePath;
        }
    }
?>