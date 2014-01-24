<?php
/**
  @file
  @brief Defines the stat tables.
*/

function hockey_boss_stat_table_edit_pps ($user) {
  return true;
}

function hockey_boss_stat_table_default_field_value ($column_name, $row_name, $player_nid) {
  // I. Retrieve the stat associate with the field.
  $field_stat_name = $column_name;
  $field_stat_year = $row_name;

  $stats = stat_get_stats_by_subject (hockey_boss_stat_PPS_TYPE, $player_nid);
  foreach ($stats as $stat) {
    if ($stat->title != $field_stat_name) {
      continue;
    }
    $stat_year = date ('Y', strtotime (stat_get_date ($stat)));
    if ($stat_year != $field_stat_year) {
      continue;
    }
    $stat_unit = stat_get_unit ($stat);
    $stat_value = stat_get_value ($stat);
    return array (
      'field_id'      => "stat_$stat->nid",
      'editable'      => true,
      'column_name'   => $column_name,
      'row_name'      => $row_name,
      'value'         => $stat_value,
      'display_value' => $stat_value,
      'suffix'        => $stat_unit
    );
  }
  // II. Create a stat node to represent the missing stat.
  $stat = new stdClass ();
  $stat->type = hockey_boss_stat_PPS_TYPE;
  node_object_prepare ($stat);
  $stat->title = $field_stat_name;
  $stat = stat_set_value ($stat, '0');
  $stat = stat_set_date ($stat, $field_stat_year . '-01-01T12:00:00');
  $stat = stat_set_subject ($stat, $player_nid);
  node_save ($stat);
  return array (
    'field_id'      => "stat_$stat->nid",
    'editable'      => true,
    'column_name'   => $column_name,
    'row_name'      => $row_name,
    'value'         => '0',
    'display_value' => '0'
  );
}

/**
*/
function hockey_boss_stat_table_submit ($form, $form_state) {
  foreach ($form_state ['input'] as $key => $value) {
    if (strpos ($key, 'stat_') !== 0) {
      continue;
    }
    $stat_nid = substr ($key, 5);
    $stat = node_load ($stat_nid);
    if (is_null ($stat)) {
      continue;
    }
    stat_set_value ($stat, $value);
    node_save ($stat);
  }
  drupal_set_message ('Stats updated.');
}

/**
  @brief Accepts a player node and returns a simple
    table form that represents the player's
    performance stats table.
  @param $player (node) the player node.
  @return (array) the stats table form.
*/
function hockey_boss_stat_table_pps ($player) {
  module_load_include ('inc', 'hockey_boss');
  module_load_include ('inc', 'hockey_boss_stat');
  module_load_include ('inc', 'stat');

  $stats = stat_get_stats_by_subject (hockey_boss_stat_PPS_TYPE, $player->nid);

  $data = array (
    'rows' => array ('2014', '2013'),
    'columns' => array (
      array (
        'column_name' => t ('Year'),
        'weight'      => 10
      ),
      array (
        'column_name' => t ('Squats'),
        'weight'      => 9
      ),
      array (
        'column_name' => t ('Sprint'),
        'weight'      => 8
    )), 
    'fields'          => array (),
    'sort_column'     => t ('Year'),
    'submit_function'               => 'hockey_boss_stat_table_submit',
    'access_function'               => 'hockey_boss_stat_table_edit_pps',
    'default_field_value_function'  => 'hockey_boss_stat_table_default_field_value',
    'default_field_value_arguments' => array ($player->nid)
  ); 
  foreach ($data ['rows'] as $row) {
    $data ['fields'][] = array (
      'column_name' => t ('Year'),
      'row_name'    => $row,
      'value'       => $row
    );
  }
  return $data;
}
