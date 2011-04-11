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


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Json_Management
*
* @see Asset_Management
*
* @author  Daniel Simmons <dsimmons@squiz.co.uk>
* @version $Revision$
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Json_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_Json_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
			'path'		=> Array(
				'added'		=> '0.1',
				'type'		=> 'text',
				'default'	=> '',
			),
			'root_object'	=> Array (
				'added'			=> '0.1',
				'type'			=> 'text',
				'default'		=> '',
				'description'	=> 'Root JSON object name for parser',
			),
			'nodes'	=> Array (
				'added'			=> '0.1',
				'type'			=> 'text',
				'default'		=> '',
				'description'	=> 'Newline-separated list of JSON object properties to retrieve',
			),
			'parameter_map'	=> Array(
				'added'			=> '0.1',
				'type'			=> 'parameter_map',
				'default'		=> Array(),
				'parameters'	=> Array(
					'parameters' => Array(
						'source_asset' => 'Source asset for JSON data'
					),
				),
		   ),
		);

	}//end constructor


}//end class

?>
