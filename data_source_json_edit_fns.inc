<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id$
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source_edit_fns.inc';

/**
* Data_Source_Json_Edit_Fns
*
* Purpose
*
*
* @author  Daniel Simmons <dsimmons@squiz.co.uk>
* @version $Revision$
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Json_Edit_Fns extends Data_Source_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Outputs the value of the user-defined nodes (attribute)
	*
	* @param Data_Source		$asset	the asset we are handling
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	public function paintJsonRootObject(Data_Source_Json $asset, Backend_Outputter $o, $prefix)
	{
		if (isset($asset->vars['root_object']['value'])) {
			echo '<pre>' . $asset->vars['root_object']['value'] . '</pre>';
		}

	}//end paintJsonUserDefinedNodes()

	/**
	* Outputs the value of the user-defined nodes (attribute)
	*
	* @param Data_Source		$asset	the asset we are handling
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	public function paintJsonUserDefinedNodes(Data_Source_Json $asset, Backend_Outputter $o, $prefix)
	{
		$json_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
		$nodes = $json_asset->getUserDefinedJsNodes();
		if (!empty($nodes)) {
			echo '<pre>';
			foreach ($nodes as $node) {
				echo $node . "\n";
			}
			echo '</pre>';
		}

	}//end paintJsonUserDefinedNodes()


	/**
	* Dumps the parsed JSON data on the details screen for debugging aid
	*
	* @param Data_Source		$asset	the asset we are handling
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	public function paintJsonData(Data_Source_Json $asset, Backend_Outputter $o, $prefix)
	{
		$json_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
		echo "<pre>";
		print_r($json_asset->getItems());
		echo "</pre>";

	}//end paintJsonData()


	/**
	* Dumps the parsed JSON data on the details screen for debugging aid
	*
	* @param Data_Source		$asset	the asset we are handling
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	public function paintJsonDataSource(Data_Source_Json $asset, Backend_Outputter $o, $prefix)
	{
		$json_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
		echo "<pre>";
		print_r($json_asset->getRawSourceData());
		echo "</pre>";

	}//end paintJsonData()


	/**
	* Paints the root node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintConnectorLink(&$asset, &$o, $prefix)
	{
		// get root links
		$connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, 'json_data_source');
		if (empty($connector_link)) {
			$connector_asset_id = NULL;
		} else {
			$connector_asset_id = $connector_link['minorid'];
		}

		if ($asset->writeAccess('links')) {
			asset_finder($prefix.'_connector_node', $connector_asset_id, Array(
				'page_rest_resource' => 'I',
				'page_standard' => 'I',
				'page_asset_listing' => 'I',
				'search_page' => 'I',
				'file' => 'I',
				'text_file' => 'I',
				'js_file' => 'I',
				)
			);
		} else {
			if (empty($connector_asset_id)) {
				echo 'No source asset selected.';
			} else {
				echo get_asset_tag_line($connector_asset_id);
			}
		}
		return TRUE;

	}//end paintConnectorLink()

	/**
	* Paints the root node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processConnectorLink(&$asset, &$o, $prefix)
	{
		$new_connector_id = @$_REQUEST[$prefix.'_connector_node']['assetid'];

		if (!$asset->writeAccess('links')) {
			trigger_error('You do not have write access to change the JSON source link', E_USER_WARNING);
			return FALSE;
		}

		$current_connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, 'json_data_source');
		$current_connector_asset_id = @$current_connector_link['minorid'];

		if (!empty($current_connector_link)) {
			// if the new connector id is not null and is equal to the current
			// connector asset id then return
			if ((!empty($new_connector_id)) && ($current_connector_asset_id === $new_connector_id)) {
				return TRUE;
			} else {
				// else means that we want to create a new link or we want to
				// clear the link
				$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_connector_link['linkid']);

				// if the new connector id is empty means we want to clear the
				// link we got nothing else to do so return TRUE
				if (empty($new_connector_id))return TRUE;
			}
		}	

		if (!empty($new_connector_id)) {
			$new_connector_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_connector_id);
			$result_id = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $new_connector_asset, SQ_LINK_NOTICE, 'json_data_source');

			if (empty($result_id)) {
			trigger_error('Could not link asset as JSON source asset', E_USER_WARNING);
			return FALSE;
		}

		}

		return TRUE;

	}//end processConnectorLink()

}//end class
?>
