<?php

$plugin = array(
  'title' => 'PostGIS',
  'schema' => 'geofield_postgis_backend_schema',
  'save' => 'geofield_postgis_backend_save',
  'types' => array('Point','LineString','Polygon','MultiPoint','MultiLineString','MultiPolygon','GeometryCollection'),
  'validate' => NULL,     //@@TODO: Additional validation callback. Takes a GeoPHP Geometry object.
  'postinstall' => NULL,  //@@TODO: Callback after installing a new field. Could be used for adding additional geospatial indexes
  'postdelete' => NULL,   //@@TODO: Callback after deleting field. Could be used for removing indexes and other cleanup.
  'requirements' => 'geofield_postgis_backend_requirements',
  'settings' => 'geofield_postgis_backend_settings',
);

function geofield_postgis_backend_schema($field) {
  //@@TODO: Geometry vs. Geography
  //@@TODO: SRID
  $settings = $field['settings']['settings']['postgis'];
  return array(
    'type' => 'blob',
    'pgsql_type' => $settings['geomtype'],
    'not null' => FALSE,
  );
}

function geofield_postgis_backend_save($geometry) {
  $unpacked = unpack('H*', $geometry->out('ewkb'));
  return $unpacked[1];
}

function geofield_postgis_backend_requirements(&$error) {
  try {
    $value = db_query('SELECT substring(PostGIS_Version() from 1 for 3)')->fetchField();
    if (empty($value)) {
      $error = 'Could not detect postGIS version';
      return FALSE;
    }

    $comparisson = version_compare($value, '1.5');
    if ($comparisson === FALSE) {
      $error = 'Could not detect postGIS version';
      return FALSE;
    }
    if ($comparisson == -1) {
      $error = 'PostGIS 1.5 or later required';
      return FALSE;
    }
    return TRUE;
  }
  catch (PDOException $e) {
    $error = 'Could not detect postGIS';
    return FALSE;
  }
}

function geofield_postgis_backend_settings($defaults) {
  $form = array();
  
  $form['fieldtype'] = array(
    '#title' => 'PostGIS Type',
    '#type' => 'select',
    '#options' => array(
      'geometry' => 'Geometry',
    ),
    '#default_value' => isset($defaults['fieldtype']) ? $defaults['fieldtype'] : 'geometry',
  );
  
  // If we are running postGIS 2.0, add an option to use geography fields
  $version = db_query('SELECT substring(PostGIS_Version() from 1 for 3)')->fetchField();
  if (version_compare($version, '2.0')) {
    $form['#options']['geography'] = 'Geography';
  }
  
  return $form;
}