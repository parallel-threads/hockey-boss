<?php
/**
  @file
  @brief Defines the functions that create the
    Stat content type and the Unit vocabulary.
*/

// ----- I. Vocabularies. -----

/// Returns an array listing the MHealthDB Contributed vocabularies.
function stat_vocabularies () {
  return array (
    'stat_unit' => array (
      'name'         => 'Unit',
      'machine_name' => 'stat_unit',
      'description'  => 'Standard measuring units used for statistics.'
  ));
}

/**
  @brief Creates a new vocabulary.
  @param $vocab (array) a vocabulary array.
*/
function _stat_create_vocabulary ($vocab) {
  if (!taxonomy_vocabulary_machine_name_load ($vocab ['machine_name'])) {
    taxonomy_vocabulary_save ((object) $vocab);
  }
}

/// Creates the Stat vocabularies.
function stat_create_vocabularies () {
  foreach (stat_vocabularies () as $vocab) {
    _stat_create_vocabulary ($vocab);
  }
}

/// Deletes the Stat vocabularies.
function stat_delete_vocabularies () {
  foreach (array_keys (stat_vocabularies ()) as $machine_name) {
    $vocab = taxonomy_vocabulary_machine_name_load ($machine_name);
    taxonomy_vocabulary_delete ($vocab->vid);
  }
}

// ----- II. Content Types -----

/// The Stat content type name.
define ('stat_STAT_TYPE', 'stat_stat');

/// Defines the Stat content type.
function stat_stat_content_type () {
  $t = get_t (); 
  return node_type_set_defaults (
    array (
      'type'        => stat_STAT_TYPE,
      'name'        => $t ('Stat'),
      'description' => $t ('Represents statistics.'),
      'help'        => $t ('Represents statistics.'),
      'has_title'   => true,
      'base'        => 'node_content',
      'custom'      => false,
      'modified'    => true,
      'locked'      => true,
      'module'      => 'stat'
  )); 
}

/// Creates the Stat content type.
function stat_create_stat_content_type () {
  module_load_include ('inc', 'stat', 'stat.field');

  $type = mhealthdb_contributed_content_type ();
  node_type_save ($type);

  stat_create_textfield        ($type, 'stat_value',   t ('Value'));
  stat_create_term_reference   ($type, 'stat_unit',    t ('Units'),   array ('stat_unit'));
  stat_create_entity_reference ($type, 'stat_subject', t ('Subject'), array ('node'));
  stat_create_date_field       ($type, 'stat_date',    t ('Date'));
}

/**
  @brief Assigns the given permissions to the role
    that has the given name.
  @param $name (string) the role name.
  @param $permissions (array) the permission names.
*/
function _stat_grant_permissions ($name, $permissions) {
  $role = user_role_load_by_name ($name);
  user_role_grant_permissions ($role->rid, $permissions);
}

/**
  @brief Returns the node id of every Stat node.
  @return (array) the node ids.
*/
function stat_get_stat_nodes () {
  $records = db_select   ('{node}', 'tbl')
             ->fields    ('tbl', array ('nid'))
             ->condition ('type', stat_STAT_TYPE)
             ->execute   ();

  $nids = array ();
  foreach ($records as $record) {
    $nids [] = $record->nid;
  }
  return $nids;
}

/// Deletes every stat node.
function stat_delete_stat_nodes () {
  foreach (stat_get_stat_nodes () as $nid) {
    node_delete ($nid);
  }
}

// ----- III. Permissions -----

/**
  @brief Returns a list of the core user roles and
    their permissions.
  @return (array) the core user roles.
*/
function stat_permissions () {
  return array (
    'authenticated user' => array (
      'create ' . stat_STAT_TYPE . ' content'
  ));
}

/**
  @brief Creates the core user roles and assigns
    them their default permissions.
*/
function stat_set_permissions () {
  foreach (stat_permissions () as $name => $permissions) {
    _stat_grant_permissions ($name, $permissions);
  }
}

