<?php
/**
  @file
  @brief Defines the functions that create the
    Stat content type and the Unit vocabulary.
*/

// ----- I. Vocabularies -----

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

/**
  @brief Defines a stat type.
  @param $machine_name (string) the stat type machine name.
  @param $name (string) the stat type name.
  @param $description (string) a description of the stat type.
  @return (object) a stat content type.
*/
function stat_get_stat_type ($machine_name, $name, $description) {
  $t = get_t (); 
  return node_type_set_defaults (
    array (
      'type'        => $machine_name,
      'name'        => $t ($name),
      'description' => $t ($description),
      'help'        => $t ($description),
      'has_title'   => true,
      'base'        => 'node_content',
      'custom'      => false,
      'modified'    => true,
      'locked'      => true,
      'module'      => 'stat'
  )); 
}

/**
  @brief Creates a stat type.
  @details This function creates a stat type and defines the core stat fields.
  @param $machine_name (string) the stat type machine name.
  @param $name (string) the stat type name.
  @param $description (string) a description of the stat type.
*/
function stat_create_stat_type ($machine_name, $name, $description) {
  // I. Create the content type.
  $type = stat_get_stat_type ($machine_name, $name, $description);
  node_type_save ($type);

  // II. Create core fields.
  simple_field_create_textfield        ($type, $machine_name . '_value',   t ('Value'));
  simple_field_create_term_reference   ($type, $machine_name . '_unit',    t ('Units'),   array ('stat_unit'), 1);
  simple_field_create_entity_reference ($type, $machine_name . '_subject', t ('Subject'), array ('node'), 1);
  simple_field_create_date_field       ($type, $machine_name . '_date',    t ('Date'));

  // III. Return the type definition.
  return $type;
}

/**
  @brief Accepts a set of stat type definitions and
    creates content types that represent each.
  @param $defs (array) the stat type definitions.
  @note The definition array must have the
    following structure:
    array (
      <machine name> => array (
        'name'          => <human readable stat type name>, // required.
        'description'   => <a stat type description>,       // required.
        'create_fields' => array (                          // optional.
          'module'   => <the name of the module whose include file will be loaded>, // optional.
          'callback' => <the callback function that accepts the type array and creates the type-specific fields> // required.
      )),
      ...
    )
*/
function stat_create_stat_types ($defs) {
  foreach ((array) $defs as $machine_name => $def) {
    // I. Create the content type and core fields.
    $type = stat_create_stat_type ($machine_name, $def ['name'], $def ['description']);

    // II. Create the type-specific fields.
    if ($def ['create_fields']) {
      if (isset ($def ['create_fields']['module'])) {
        module_load_include ('inc', $def ['create_fields']['module']);
      }
      call_user_func ($def ['create_fields']['callback'], $type);
    }
  }
}

/**
  @brief Accepts a set of stat type definitions and deletes them.
  @param $defs (array) the stat type definitions.
  @see stat_create_stat_types () for details about the structure of the stat type definitions array.
*/
function stat_delete_stat_types ($defs) {
  stat_delete_node_types (array_keys ((array) $defs));
}

// ----- III. Node Operations -----

/*
  @brief Returns the node id of every node that has the given type.
  @param $type (string) the node type.
  @return (array) the node ids.
*/
function stat_get_nodes ($type) {
  $records = db_select   ('{node}', 'tbl')
             ->fields    ('tbl', array ('nid'))
             ->condition ('type', $type)
             ->execute   ();

  $nids = array ();
  foreach ($records as $record) {
    $nids [] = $record->nid;
  }
  return $nids;
}

/**
  @brief Deletes every node that belongs to the given type.
  @param $type (string) the node type.
*/
function stat_delete_nodes ($type) {
  foreach (stat_get_nodes ($type) as $nid) {
    node_delete ($nid);
  }
}

/**
  @brief Deletes every node that belongs to a given type and the type itself.
  @param $type (string) the node type.
*/
function stat_delete_node_type ($type) {
  // I. Delete the node type nodes.
  stat_delete_nodes ($type);

  // II. Delete the node type.
  node_type_delete ($type);
}

/**
  @brief Accepts a list of node types and deletes them and their nodes.
  @param $types (array) the node types.
*/
function stat_delete_node_types ($types) {
  foreach ((array) $types as $type) {
    stat_delete_node_type ($type);
  }
}

/**
  @brief Accepts a stat node and returns its value. 
  @param $node (object) the stat node.
  @return (string) the stat value.
*/
function stat_get_value ($node) {
  return $node->{$node->type . '_value'}['und'][0]['value'];
}

/**
  @brief Accepts a stat node and returns its unit's term id.
  @param $node (object) the stat node.
  @return (int) the unit term id.
*/
function stat_get_unit_tid ($node) {
  return $node->{$node->type . '_unit'}['und'][0]['tid'];
}

/**
  @brief Accepts a stat node and returns its unit's term.
  @param $node (object) the stat node.
  @return (object) the unit term.
*/
function stat_get_unit_term ($node) {
  return taxonomy_term_load ($stat_get_unit_tid ($node));
}

/**
  @brief Accepts a stat node and returns its unit's name.
  @param $node (object) the stat node.
  @return (string) the unit name.
*/
function stat_get_unit ($node) {
  return stat_get_unit_term ($node)->title;
}
