<?php
    $selfURL =  "http://" . $_SERVER['SERVER_NAME'] . $_SERVER['SCRIPT_NAME'];//URL pointing to this script (for automatic recursive calls)
    
    //requested file
    if (isset($_GET["file"])) {
        $reqFile = $_GET["file"];//Path of the requsted file (relative within DereGlobus project)
    }else {
        $reqFile = "DereGlobus_SQLite/DereGlobus.kml";//Default: Top level file of DereGlobus
    }
    
    //language
    if (isset($_GET["language"])) {
        if($_GET["language"] != "german") {//No translation necessary for german
            $language = $_GET["language"];//Requested language for translation
        }
    }
    
    //translation file
    if(isset($language)){//No translation necessary if no language requested
        $translationDocument = new DOMDocument('1.0', 'UTF-8');
        $requFile_pathInfo = pathinfo($reqFile);
        $translationFile = $requFile_pathInfo['dirname'] . "/" . $requFile_pathInfo['filename'] . '_translation.xml';
        if(!file_exists($translationFile) || !$translationDocument->load($translationFile)) {
            $translationDocument = NULL;
        }
    }
    
    //load the KML/XML document
    $dom = new DOMDocument('1.0', 'UTF-8');
    $dom->load($reqFile);
    
    //For test puropses
    //$testXPath = new DOMXPath($dom);
    //$testXPath->registerNamespace("x", "http://www.opengis.net/kml/2.2");
    //$searchNodes = $testXPath->query('x:Document', $dom->documentElement);
    //$testNode = $dom->createElement('Test', $dom->documentElement->namespaceURI . " " . $dom->documentElement->nodeName);
    //$dom->documentElement->appendChild($testNode);
    
    $root = $dom->documentElement;
    processNode($dom, $root, $selfURL, $translationDocument, $language);
    
    
    
    //For test puropses: appends a node with the extracted self URL
    //$testNode = $dom->createElement('TestSelfUrl', $selfURL);
    //$root->appendChild($testNode);
    
    $kmlOutput = $dom->saveXML();
    header('Content-type: application/vnd.google-earth.kml+xml');
    header('Content-Disposition: attachment; filename=' . $reqFile);
    echo $kmlOutput;
    
    function processNode (&$dom, &$node, &$selfURL, &$translationDocument, &$language) {
        //TODO: Maybe it is possible to skip some nodes which are not relevant
        foreach ($node->childNodes as $childNode) {
            if($childNode->nodeName == "NetworkLink"){
                replaceNetworkLinkContent($dom, $childNode, $selfURL, $translationDocument, $language);
                
                //For test purposes only: Mark each NetworkLink which has been found
                //$testNode = $dom->createElement('FoundNetworkLink', 'FoundNetworkLink');
                //$childNode->appendChild($testNode);
            }elseif ($childNode->nodeName == "ExtendedData") {//Look for extended data
                //Process nodes which have a ExtendedData structure  belonging to the DereGlobus procjet attached
                //Attention: If this processing should happen for nodes explicitly captured above, the capture above is responsible for executing the function call, this will not trigger
                processNodeWithExtendedData($dom, $node, $childNode, $translationDocument, $language);
            }else{
                processNode($dom, $childNode, $selfURL, $translationDocument, $language);//Recursive call
            }
        }
    }
    
    function processNodeWithExtendedData(&$dom, &$node, &$extendedDataNode, &$translationDocument, &$language) {
        //For all the ExtendedData
        foreach ($extendedDataNode->childNodes as $extendedDataChildNode) {
            if ($extendedDataChildNode->localName == "translationStructure" && $extendedDataChildNode->namespaceURI == "http://www.dereglobus.orkenspalter.de") {
                //Perform translation replacements
                $translationStructureNode = $extendedDataChildNode;
                if(isset($translationDocument)){
                    $documentXPath = new DOMXPath($dom);
                    $documentXPath->registerNamespace("x", $dom->documentElement->namespaceURI);//TODO: Make automatic namespace recognition
                    $translationDocumentXPath = new DOMXPath($translationDocument);
                    performTranslation($documentXPath, $node, $translationStructureNode, $translationDocumentXPath, $language);
                }
                
                //For test purposes only: Mark each translationStructure which has been found
                //$testNode = $dom->createElement('FoundTranslationStructure', 'FoundTranslationStructure');
                //$translationStructureNode->appendChild($testNode);
            }
        }
        //For test purposes only: Mark each translationStructure which has been found
        //$testNode = $dom->createElement('FoundExtendedData', 'FoundExtendedData');
        //$extendedDataNode->appendChild($testNode);
    }
    
    function replaceNetworkLinkContent(&$dom, &$networkLinkNode, &$selfURL, &$translationDocument, &$language) {
        //Collect Information
        foreach ($networkLinkNode->childNodes as $childNode) {
            //For test purposes only: Mark each node
            //$testNode = $dom->createElement('Information', $childNode->nodeName);
            //$childNode->appendChild($testNode);
            
            if ($childNode->nodeName == "ExtendedData") {
                //Perform general operations
                processNodeWithExtendedData($dom, $networkLinkNode, $childNode, $translationDocument, $language);
                
                //Search for extended data relvant for the network node specific replacements
                foreach ($childNode->childNodes as $extendedDataChildNode) {
                    if($extendedDataChildNode->localName == "FilePath" && $extendedDataChildNode->namespaceURI == "http://www.dereglobus.orkenspalter.de"){
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
        //Perform link replacements
        if(isset($filePath)){
            if(isset($language)){
                $languageQuery =  "&amp;" . "language=" . $language;
            }else{
                $languageQuery = "";
            }
            $linkNode_hrefNode->nodeValue = ($selfURL . "?" . "file=" . $filePath . $languageQuery);
        }
    }
    
    function performTranslation(&$documentXPath, &$nodeToTransalte, &$translationStructureNode, &$translationDocumentXPath, &$language){
        foreach ($translationStructureNode->childNodes as $transChildNode) {
            if($transChildNode->nodeType == XML_ELEMENT_NODE){
                if($transChildNode->localName == "translationMarker" && $transChildNode->namespaceURI == "http://www.dereglobus.orkenspalter.de") {
                    //Perform transaltion according to this marker
                    $translationName = $transChildNode->getAttribute("translationName");
                    $translationNodes = $translationDocumentXPath->query($translationName . "/" . $language);
                    if($translationNodes == FALSE || $translationNodes->count() == 0){
                        $translationNodes = $translationDocumentXPath->query($translationName . "/" . "english");//Default to english
                    }
                    if($translationNodes == FALSE || $translationNodes->count() == 0){
                        //No translation possible, continue with non-translated content
                    }else{
                        $translation = $translationNodes->item(0)->nodeValue;
                        $nodeToTransalte->nodeValue = $translation;//Replace node value with translation
                        
                        //For test purposes only
                        //$testNode = $documentXPath->document->createElement('TestTranslation', $translation);
                        //$nodeToTransalte->appendChild($testNode);
                    }
                }else{
                    //Recursive call for child nodes which are present in the translation structure
                    $toTransChildNode = $documentXPath->query("x:" . $transChildNode->nodeName, $nodeToTransalte)->item(0);
                    performTranslation($documentXPath, $toTransChildNode, $transChildNode, $translationDocumentXPath, $language);
                    
                    //For test purposes only
                    //$testNode = $documentXPath->document->createElement('Info', $nodeToTransalte->nodeName . " " . $toTransChildNode->nodeName);
                    //$transChildNode->appendChild($testNode);
                }
            }
        }
    }
?>