<?php
//=======================================
function getConfigValue($sid,$par)
//======================================= 
{
    $value = 'void';
    $filename = $sid.'.conf';
    if (file_exists($filename)) 
    {

        $handle = fopen($filename, "r");
        if ($handle) 
        {    
            while (($line = fgets($handle)) !== false) 
            {
                $line = trim($line);
                $pos = strpos($line, $par);
                if($pos !== null)
                {
                    sscanf($line,"%s %s",$key,$value);
                }
            }
            fclose($handle);
        }
        return($value);
    }
    else
    {
        $handle = fopen($filename, "w");
        if ($handle) 
        {
            fwrite($f, $data);
        }
    }
    }
}

//==============================================================================
// Main program
//==============================================================================


if(isset($_GET['sid']) && isset($_GET['config']))
{
    $sid    = $_GET['sid'];
    $config = $_GET['config'];
    $res = getConfigValue($sid,$config);
    echo("$sid $config $res");
}

?>