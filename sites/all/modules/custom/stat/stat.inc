<?php
/**
  @file
  @brief Defines the functions that create the
    Stat content type and the Unit vocabulary.
*/

// ----- I. Stat Types -----

/**
  @brief Creates a stat type.
  @param $stat_type (array) a stat type.
  @note Stat type arrays must have the following structure:
    array (
      'name'              => <the name of the stat type>
      'description'       => <a stat type description>
      'node_type'         => <the machine name of the node type used to represent stats>
      'subject_node_type' => <the machine name of the node type referenced by the subject field>
    )
*/
function stat_create_stat_type ($stat_type) {
  watchdog (
    'stat',
    '[stat_create_stat_type] stat type: <pre>@stat_type</pre>',
    array (
      '@stat_type' => print_r ($stat_type, true)
  ));
  drupal_write_record ('stat_stat_type', $stat_type);
  stat_create_stat_type_node_type ($stat_type);
};

/**
  @brief Creates the given stat types.
  @param $stat_types (array) the stat types.
*/
function stat_create_stat_types ($stat_types) {
  foreach ((array) $stat_types as $stat_type) {
    stat_create_stat_type ($stat_type);
  }
}

/**
  @brief Returns a database query object that will retrieve stat type records.
  @return (SelectQuery) the query object.
*/
function stat_stat_type_select () {
  return db_select ('{stat_stat_type}', 'tbl')
         ->fields  ('tbl', array ('name', 'description', 'node_type', 'subject_node_type'));
}

/**
  @brief Accepts a stat type database record and returns a stat type array.
  @param $record (object) the record.
  @return (array) the stat type.
*/
function stat_get_stat_type_from_record ($record) {
  return array (
    'name'              => $record->name,
    'description'       => $record->description,
    'node_type'         => $record->node_type,
    'subject_node_type' => $record->subject_node_type
  );
}

/**
  @brief Accepts stat type database records and returns the stat type arrays.
  @param $records (DatabaseConnection_mysql)
  @return (array) the stat types.
*/
function stat_get_stat_types_from_records ($records) {
  $stat_types = array ();
  foreach ($records as $record) {
    $stat_types [] = stat_get_stat_type_from_record ($record);
  }
  return $stat_types;
}

/**
  @brief Returns all of the stat types.
  @return (array) the stat types.
*/
function stat_get_all_stat_types () {
  return stat_get_stat_types_from_records (
    stat_stat_type_select ()
    ->execute ());
}

/**
  @brief Returns the stat types that use the given node type to represent stats.
  @param $node_type (string) the node type.
  @return (array) the stat types.
*/
function stat_get_stat_types_by_node_type ($node_type) {
  return stat_get_stat_types_from_records (
    stat_stat_type_select ()
    ->condition ('subject_node_type', $node_type)
    ->execute   ());
}

/**
  @brief Implements hook_node_type_delete ().
  @details
    Deletes the stat type associated with the node
    type that is being deleted.
*/
function stat_node_type_delete ($info) {
  stat_delete_stat_types (stat_get_stat_types_by_node_type ($info->type));
}

/**
  @brief Deletes the given stat type.
  @param $stat_type (array) the stat type.
*/
function stat_delete_stat_type ($stat_type) {
  watchdog (
    'stat',
    '[stat_delete_stat_type] stat type: <pre>@stat_type</pre>',
    array (
      '@stat_type' => print_r ($stat_type, true)
  ));

  // I. Delete the stat type's node type and stat nodes.
  stat_delete_stat_type_node_type ($stat_type);

  // II. Delete the stat type's database record.
  db_delete ('{stat_stat_type}')
  ->condition ('node_type', $stat_type ['node_type'])
  ->execute   ();
}

/**
  @brief Deletes the given stat types.
  @param $stat_types (array) the stat types.
*/
function stat_delete_stat_types ($stat_types) {
  foreach ((array) $stat_types as $stat_type) {
    stat_delete_stat_type ($stat_type);
  }
}

/// Deletes all of the stat types.
function stat_delete_all_stat_types () {
  stat_delete_stat_types (stat_get_all_stat_types ());
}

// ----- II. Stat Type Node Types -----

/**
  @brief Defines a stat type's node type.
  @param $stat_type (array) a stat type.
  @return (object) a content type.
*/
function stat_get_stat_type_node_type ($stat_type) {
  $t = get_t (); 
  return node_type_set_defaults (
    array (
      'type'        => $stat_type ['node_type'],
      'name'        => t ($stat_type ['name']),
      'description' => t ($stat_type ['description']),
      'help'        => t ($stat_type ['description']),
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
  @details This function creates a stat type node type and defines the core stat fields.
  @param $stat_type (array) the stat type.
*/
function stat_create_stat_type_node_type ($stat_type) {
  // I. Create the content type.
  $stat_type_node_type = stat_get_stat_type_node_type ($stat_type);
  node_type_save ($stat_type_node_type);

  // II. Create core fields.
  simple_field_create_textfield        ($stat_type_node_type, $stat_type ['node_type'] . '_value',   t ('Value'));
  simple_field_create_textfield        ($stat_type_node_type, $stat_type ['node_type'] . '_unit',    t ('Units'));
  simple_field_create_entity_reference ($stat_type_node_type, $stat_type ['node_type'] . '_subject', t ('Subject'), is_null ($stat_type ['subject_node_type']) ? array () : array ($stat_type ['subject_node_type']), 'node', 1);
  simple_field_create_date_field       ($stat_type_node_type, $stat_type ['node_type'] . '_date',    t ('Date'));

  // III. Call hook_stat_create_<stat_type_node_type>_stat_type_node_type () to add custom fields.
  $hook = 'stat_create_' . $stat_type ['node_type'] . '_stat_type_node_type';
  watchdog (
    'stat',
    '[stat_create_stat_type_node_type] hook: @hook',
    array (
      '@hook' => $hook
  ));
  module_invoke_all ($hook, $stat_type_node_type);

  // IV. Return the type definition.
  return $stat_type_node_type;
}

/**
  @brief Accepts a set of stat types and
    creates content types that represent each.
  @param $stat_types (array) the stat types.
*/
function stat_create_stat_types_node_types ($stat_types) {
  foreach ((array) $stat_types as $stat_type) {
    stat_create_stat_type_node_type ($stat_type);
  }
}

/**
  @brief Accepts a stat type and deletes its node type.
  @param $stat_type (array) the stat type.
*/
function stat_delete_stat_type_node_type ($stat_type) {
  stat_delete_node_type ($stat_type ['node_type']);
}

/**
  @brief Accepts a set of stat types and deletes them.
  @param $stat_types (array) the stat types.
*/
function stat_delete_stat_types_node_types ($stat_types) {
  foreach ((array) $stat_types as $stat_type) {
    stat_delete_stat_type_node_type ($stat_type);
  }
}

// ----- III. Stat Nodes -----

/**
  @brief Accepts a information about a stat and returns a node object with the core fields set.
  @param $stat_type (array) the stat type.
  @param $name (string) the human readable stat name.
  @param $value (string) the stat value.
  @param $unit  (string) the units of measure.
  @param $date  (string) the stat date.
  @param $subject_nid (int) the stat subject.
  @return (node) the stat node with core fields set.
*/
function stat_create_stat_node ($stat_type, $name, $value, $unit, $date, $subject_nid) {
  // I. Create the stat node and set the core fields.
  $stat = new stdClass ();
  $stat->type = $stat_type ['node_type'];
  node_object_prepare ($stat);
  $stat->title = $name;
  stat_set_value    ($stat, $value);
  stat_set_unit     ($stat, $unit);
  stat_set_date     ($stat, $date);
  stat_set_subject  ($stat, $subject_nid);

  // II. Call hook_stat_create_<stat_type_node_type>_stat_node () to set custom field values.
  module_invoke_all ('stat_create_' . $stat_type ['node_type'] . '_stat_node', $stat);

  watchdog (
    'stat',
    '[stat_create_stat_node] stat: <pre>@stat</pre>',
    array (
      '@stat' => print_r ($stat, true)
  ));

  // III. Saves the newly created stat node.
  node_save ($stat);
  return $stat;
}

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
  @brief Accepts an entity id and returns those
    stats that reference the entity that has the
    given id.
*/
function stat_get_stats_by_subject_nid ($type, $entity_id) {
  $stats = array ();
  foreach ((array) stat_get_nodes ($type) as $stat_nid) {
    $stat = node_load ($stat_nid);
    if ($entity_id == stat_get_subject_nid ($stat)) {
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
function stat_get_subject_nid ($node) {
  return $node->{$node->type . '_subject'}['und'][0]['target_id'];
}

/**
  @brief Accepts a stat node and a subject id and updates the stat value.
  @param $node (object) the stat node.
  @param $subject_id (string) the subject entity id.
*/
function stat_set_subject ($node, $subject_id) {
  $node->{$node->type . '_subject'}['und'][0]['target_id'] = $subject_id;
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
*/
function stat_set_date ($node, $date) {
  $node->{$node->type . '_date'}['und'][0] = array (
    'value'     => $date,
    'date_type' => 'date'
  );
}

/**
  @brief Accepts a stat node and returns its year.
  @param $node (object) the stat node.
  @return (int) the year.
*/
function stat_get_year ($node) {
  // return date ('Y', strtotime (stat_get_date ($node)));
  $date = stat_get_date ($node);
  $year = date ('Y', strtotime ($date));
  return $year + 1;
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
*/
function stat_set_value ($node, $value) {
  $node->{$node->type . '_value'}['und'][0]['value'] = $value;
}

/**
  @brief Accepts a stat node and returns its units. 
  @param $node (object) the stat node.
  @return (string) the stat units.
*/
function stat_get_unit ($node) {
  return $node->{$node->type . '_unit'}['und'][0]['value'];
}

/**
  @brief Accepts a stat node and a unit and updates the stats unit.
  @param $node (object) the stat node.
  @param $unit (string) the stat unit.
*/
function stat_set_unit ($node, $unit) {
  $node->{$node->type . '_unit'}['und'][0]['value'] = $unit;
}
