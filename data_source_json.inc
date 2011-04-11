<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of	|
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id$
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Data_Source_Json
*
* Purpose
*
*
* @author  Daniel Simmons <dsimmons@squiz.co.uk>
* @version $Revision$
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Json extends Data_Source
{

	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['json_data_source'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = TRUE|FALSE
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		// any link is allowed
		$allowed_link['asset']['card'] = 'M';
		$allowed_link['asset']['exclusive'] = FALSE;

		$links[SQ_LINK_TYPE_1] = $allowed_link;
		$links[SQ_LINK_TYPE_2] = $allowed_link;
		$links[SQ_LINK_TYPE_3] = $allowed_link;
		$links[SQ_LINK_NOTICE] = $allowed_link;

		return $links;

	}//end _getAllowedLinks()


	/**
	* Function to parse JSON data and return its results.
	*
	* @param boolean $reencode_attrs whether or not to re-encode shadow asset
	*                                attributes
	*
	* @return array
	* @access public
	*/
	function getItems()
	{
		// Get the source data
		$json_data = $this->getRawSourceData();
		if (empty($json_data)) {
			return Array();
		}

		// Run data through the parser
		$results = $this->_parse($json_data);

		// For the top-level shadow asset attributes, if there are any arrays
		// remaining, re-encode them to JSON so that the value of the attribute
		// becomes the JSON data (for user visibility)
		foreach ($results as &$asset) {
			foreach ($asset as &$attr) {
				if (is_array($attr)) {
					$attr = self::_encodeJson($attr);
				}
			}
		}

		return $results;

	}//end getItems()


	/**
	* Fetchs the data from an asset ID's contents or local path/URI.
	*
	* @param string	$path	the path to the Json feed
	*
	* @return array
	* @access public
	*/
	public function getRawSourceData()
	{
		$source = $this->_getSource();

		// Check if source is an asset ID
		if (substr($source, 0, 2) === 'a=') {
			$source_asset = substr($source, 2);
			$data = $this->_getAssetContents($source_asset);

		// Otherwise it's a URI
		} else {
			$data = file_get_contents($source);
		}

		return $data;

	}//end getRawSourceData()


	/**
	* Returns an array of the result of JSON parsing
	*
	* @return array
	* @access public
	*/
	function getResultSet()
	{
		// Get source string (to be used as cache key)
		$source = $this->_getSource();

		if (!empty($source)) {

			// Check the local cache
			if (!isset($this->_tmp[$source])) {

				// Try from the system cache
				$result = parent::getResultSet($source);

				if ($result !== FALSE) {
					$this->_tmp[$source] = $result;
				} else {
					$this->_tmp[$source] = $this->getItems();
					parent::setResultSet($this->_tmp[$source], $source);
				}
			}
		}
		
		return (!empty($source)) ? $this->_tmp[$source] : Array();

	}//end getResultSet()


	/**
	* Get an array of the user-defined list of JSON object property or array
	* values to cherry-pick from the JSON data.
	*
	* @return array
	* @access public
	*/
	function getUserDefinedJsNodes()
	{
		// User-specified object parameters/array values to pull out
		$cherry_pick_nodes = $this->attr('nodes');
		replace_global_keywords($cherry_pick_nodes);

		return preg_split("/[\s,]+/", $cherry_pick_nodes);

	}//end getUserDefinedJsNodes()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{
		// Get the source data
		$original_json_data = $this->getRawSourceData();

		// Run data through the parser
		$results = $this->_parse($original_json_data);

		// Re-encode data into JSON
		$output_json = self::_encodeJson($results);

		// For security, JSON output must have the correct content-type header (to
		// prevent XSS attacks)
		header('Content-type: application/json');

		echo self::_formatJsonText($output_json);

	}//end printBody()

	/**
	* Convert the path of an object/array value to an array of key values.
	*
	* @return mixed
	* @access private
	*/
	private static function _convertJsPathToArray($js_string)
	{
		return explode('.', preg_replace("/\[(\d)\]/", ".$1", $js_string));

	}//end _convertJsPathToArray()


	/**
	* Decodes the specified JSON data. Returns an array.
	*
	* @param string	$json	JSON data
	*
	* @return array
	* @access public
	*/
	private function _decodeJson($json_data)
	{
		// Prefer to use inbuilt PHP 5.3 json functions (or PHP extension)
		if (function_exists('json_decode')) {
			$result = json_decode($json_data, true);
		} else {
			require_once 'Services/JSON.php';
			$json = new Services_JSON(SERVICES_JSON_LOOSE_TYPE);
			$result = $json->decode($json_data);
		}

		if ($result === NULL) {
			trigger_error($this->_getJsonParseError() . ": " . $json_data, E_USER_WARNING);
		}

		return $result;

	}//end _decodeJson()


	/**
	* Encodes the specified array into a JSON string.
	*
	* @param array	$arr	Array to be encoded
	*
	* @return string
	* @access public
	*/
	private function _encodeJson($arr)
	{
		// Prefer to use inbuilt PHP 5.3 json functions (or PHP extension)
		if (function_exists('json_encode')) {
			$result = json_encode($arr);
		} else {
			require_once 'Services/JSON.php';
			$json = new Services_JSON();
			$result = $json->encodeUnsafe($arr);

			if ($json->isError($result)) {
				trigger_error($result->message, E_USER_WARNING);
				$result = NULL;
			}
		}

		return $result;

	}//end decodeJson()


   /**
	* Indents a flat JSON string to make it more human-readable.
	*
	* @param string $json The original JSON string to process
	* @return string Indented version of the original JSON string
	* @access private
	*/
   private static function _formatJsonText($json)
   {
	   $pos = 0;
	   $indent = '	';
	   $quoting = FALSE;
	   $result = '';

	   for ($i = 0; $i <= strlen($json); $i++) {
		   $char = substr($json, $i, 1);
		   if ($char == '"') $quoting = !$quoting;

		   if (!$quoting) {
			   if($char == '}' || $char == ']') {
				   $result .= "\n";
				   $pos --;
				   for ($j = 0; $j < $pos; $j++) $result .= $indent;
			   }
		   }

		   $result .= $char;

		   if (!$quoting) {
			   if ($char == ',' || $char == '{' || $char == '[') {
				   $result .= "\n";
				   if ($char == '{' || $char == '[') $pos ++;
				   for ($j = 0; $j < $pos; $j++) $result .= $indent;
			   }
		   }
	   }

	   return $result;

   }//end _formatJsonText()


	/**
	* Checks whether the specified array is an associative array or not. This is
	* required to check if parsed JSON was an object or an array.
	*
	* @return boolean
	* @access private
	*/
	private static function _isAssociativeArray($a)
	{
		foreach (array_keys($a) as $key) {
			if (is_string($key)) {
				return true;
			}
		}

	}//end _isAssociativeArray()


	/**
	* Return the value of the specified path of nested associative arrays,
	* represented by a JS path string.
	*
	* @return mixed
	* @access private
	*/
	private static function _getArrayValueByJsPath($subject, $js_path_string)
	{
		$js_path = self::_convertJsPathToArray($js_path_string);
		return self::_getArrayValueByKeyPath($subject, $js_path);
	}


	/**
	* Returns the value of nested associative arrays by the specified key path.
	*
	* @return mixed
	* @access private
	*/
	private static function _getArrayValueByKeyPath($subject, $key_path)
	{
		if (is_array($subject)) {

			// Key path still has more than a single key, we need to recurse further down	
			if (count($key_path) > 1) {
				$next_level = array_shift($key_path);
				if (array_key_exists($next_level, $subject)) {
					return self::_getArrayValueByKeyPath($subject[$next_level], $key_path);
				}
			} else if ((count($key_path) === 1) && array_key_exists($key_path[0], $subject)) {
				// We've reach the bottom level - return the value
				return $subject[$key_path[0]];
			}
		}

		// Array value doesn't exist at this key path
		return FALSE;

	}//end _getArrayValueByKeyPath()


	/**
	* Retrieves the asset contents from the specified asset ID.
	*
	* @param string $assetid the asset ID of the asset contents to get
	*
	* @return mixed
	* @access private
	*/
	private function _getAssetContents($assetid, $type_code='')
	{
		$asset_contents = '';
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);

		if ($asset == NULL) {
			trigger_error('Unable to load asset: ' . $assetid, E_USER_WARNING);
			return NULL;
		}

		// Important to require current user to have read permission on the source
		// asset, because it can be dynamically changed if using parameter map
		if (!$asset->readAccess()) {
			trigger_error('No read permission for asset: ' . $assetid, E_USER_WARNING);
			return NULL;
		}

		ob_start();
		$asset->printBody();
		$asset_contents = ob_get_contents();
		ob_end_clean();

		return $asset_contents;

	}//end _getAssetContents()


	/**
	* Get the last JSON parse error from the json_decode function.
	*
	* @return string
	* @access public
	*/
	private function _getJsonParseError()
	{
		$error = NULL;

		// Only available in PHP 5.3
		if (function_exists('json_last_error')) {

			$error = json_last_error();

			switch($error)
			{
				case JSON_ERROR_NONE:
					return "No error has occurred";
					break;
				case JSON_ERROR_DEPTH:
					return "The maximum stack depth has been exceeded";
					break;
				case JSON_ERROR_CTRL_CHAR:
					return "Control character error, possibly incorrectly encoded";
					break;
				case JSON_ERROR_STATE_MISMATCH:
					return "Invalid or malformed JSON";
					break;
				case JSON_ERROR_SYNTAX:
					return "Syntax error";
					break;
			}
		}

		return "JSON parse error";

	}//end _getJsonParseError()


	/**
	* Determines the source asset ID or URI based on attribute settings and
	* parameter map values.
	*
	* @return string
	* @access public
	*/
	private function _getSource()
	{
		// Firstly check if replacement asset ID is specified in parameter map
		$parameter_map = $this->getAttribute('parameter_map');
		$parameter_map_source_asset = $parameter_map->getParameterValue('source_asset');
		if (!empty($parameter_map_source_asset)) {

			// If specified value isn't a valid asset ID, don't try to use it; try and
			// fall back to one of the set attribute values
			if (assert_valid_assetid($parameter_map_source_asset, '', true, false)) {
				return 'a=' . $parameter_map_source_asset;
			}
		}

		// Check if source asset is specified
   		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, 'json_data_source');
   		if (!empty($link)) {
   			return 'a=' . $link['minorid'];
   		}

		// Try to get data from plain path/URI attribute
		$uri = $this->attr('path');
		if (!empty($uri)) {
			replace_global_keywords($uri);
			return $uri;
		}

		return '';

	}//end _getSource()


	/**
	* The parser. Where all the magic happens. The source data is decoded from
	* JSON into PHP arrays, and the JSON objects are extracted as per the attribute
	* settings of this asset.
	*
	* @return array
	* @access private
	*/
	private function _parse($json_data)
	{
		// Decode the JSON
		$results = self::_decodeJson($json_data);
		if (empty($results)) {
			return Array();
		}

		// If a root object location is specified, drill down to that
		$root_object = $this->attr('root_object');
		replace_global_keywords($root_object);
		if (!empty($root_object)) {
			$results = self::_getArrayValueByJsPath($results, $root_object);
		}

		// If the parsed JSON is just a single object, wrap it in an array
		if (self::_isAssociativeArray($results)) {
			$results = Array($results);
		}

		// User-specified object parameters/array values to pull out of the JSON
		$cherry_pick_nodes = $this->getUserDefinedJsNodes();
		if (!empty($cherry_pick_nodes)) {

			foreach ($results as &$asset) {

				foreach ($cherry_pick_nodes as $attr) {
					$attr = trim($attr);
					$value = self::_getArrayValueByJsPath($asset, $attr);
	
					if ($value !== FALSE) {
						// Replace foo[1] with foo_1 as square brackets won't work in keywords
						if (strpos($attr, '[')) {
							$attr = preg_replace("/\[(\d)\]/", "_$1", $attr);
						}
						$asset[$attr] = $value;
					}
				}
			}
		}

		return $results;

	}//end _getParsedSourceData()


}//end class

?>
