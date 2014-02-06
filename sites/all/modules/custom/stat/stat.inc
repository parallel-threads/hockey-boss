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

/// Returns the Stat Unit vocabulary.
function stat_get_unit_vocab () {
  return taxonomy_vocabulary_machine_name_load ('stat_unit');
}

/// Returns the Stat Unit vocabulary terms.
function stat_get_unit_terms () {
  $vocab = stat_get_unit_vocab ();
  return taxonomy_get_tree ($vocab->vid);
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
  @param $subject_type (string) the machine name of the entity type referenced by the subject field.
*/
function stat_create_stat_type ($machine_name, $name, $description, $subject_type = null) {
  // I. Create the content type.
  $type = stat_get_stat_type ($machine_name, $name, $description);
  node_type_save ($type);

  // II. Create core fields.
  simple_field_create_textfield        ($type, $machine_name . '_value',   t ('Value'));
  simple_field_create_term_reference   ($type, $machine_name . '_unit',    t ('Units'),   array ('stat_unit'), 1);
  simple_field_create_entity_reference ($type, $machine_name . '_subject', t ('Subject'), is_null ($subject_type) ? array () : array ($subject_type), 'node', 1);
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
  @brief Accepts an entity id and returns those
    stats that reference the entity that has the
    given id.
*/
function stat_get_stats_by_subject ($type, $entity_id) {
  $stats = array ();
  foreach ((array) stat_get_nodes ($type) as $stat_nid) {
    $stat = node_load ($stat_nid);
    if ($entity_id == stat_get_subject ($stat)) {
      $stats[] = $stat;
    }
  }
  return $stats;
}

/**
  @brief Accepts a stat node and returns its subject.
  @param $node (object) the stat node.
  @return (int) the subject entity id.
*/
function stat_get_subject ($node) {
  return $node->{$node->type . '_subject'}['und'][0]['target_id'];
}

/**
  @brief Accepts a stat node and a subject id and updates the stat value.
  @param $node (object) the stat node.
  @param $subject_id (string) the subject entity id.
*/
function stat_set_subject ($node, $subject_id) {
  $node->{$node->type . '_subject'}['und'][0]['target_id'] = $subject_id;
  return $node;
} 

/**
  @brief Accepts a stat node and returns its date.
  @param $node (object) the stat node.
  @return (string) the date.
*/
function stat_get_date ($node) {
  return $node->{$node->type . '_date'}['und'][0]['value'];
}

/**
  @brief Accepts a stat node and a date and updates the date value.
  @param $node (object) the stat node.
  @param $date (string) the date value.
  @return (object) the modified stat node.
*/
function stat_set_date ($node, $date) {
  $node->{$node->type . '_date'}['und'][0] = array (
    'value'     => $date,
    'date_type' => 'date'
  );
  return $node;
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
  @brief Accepts a stat node and a value and updates the stats value.
  @param $node (object) the stat node.
  @param $value (string) the stat value.
  @return (object) the modified stat node.
  @note The stat object will need to be saved using node_save after this operation is performed.
*/
function stat_set_value ($node, $value) {
  $node->{$node->type . '_value'}['und'][0]['value'] = $value;
  return $node;
}

/**
  @brief Accepts a stat node and returns its unit's term id.
  @param $node (object) the stat node.
  @return (int) the unit term id.
*/
function stat_get_unit_tid ($node) {
  $field_name = $node->type . '_unit';
  $field = $node->{$field_name};
  return isset ($field ['und']) ? $field ['und'][0]['tid'] : null;
}

/**
  @brief Accepts a stat node and returns its unit's term.
  @param $node (object) the stat node.
  @return (object) the unit term.
*/
function stat_get_unit_term ($node) {
  $tid = stat_get_unit_tid ($node);
  return is_null ($tid) ? null : taxonomy_term_load ($tid);
}

/**
  @brief Accepts a stat node and returns its unit's name.
  @param $node (object) the stat node.
  @return (string) the unit name.
*/
function stat_get_unit ($node) {
  $term = stat_get_unit_term ($node);
  return is_null ($term) ? '' : $term->name;
}

/**
  @brief Accepts a stat node and a unit term id and sets the unit term id.
  @param $node (object) the stat node.
  @param $tid (int) the term id.
  @return (object) the modified stat node.
*/
function stat_set_unit_tid ($node, $tid) {
  $node->{$node->type . '_unit'}['und'][0]['tid'] = $tid;
  return $node;
}

function stat_unset_unit_tid ($node) {
  $node->{$node->type . '_unit'}['und'] = array ();
  return $node;
}
