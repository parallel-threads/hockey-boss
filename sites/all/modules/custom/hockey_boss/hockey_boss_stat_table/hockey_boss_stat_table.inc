<?php
/**
  @file
  @brief Defines the stat tables.
*/

/**
  @brief the access callback function for the
    Player Performance Stat table.
*/
function hockey_boss_stat_table_pps_access ($user) {
  return ($user->uid == 1); 
}

/**
  @brief The default field value callback function
    for the Player Performance Stat table.
*/
function hockey_boss_stat_table_pps_default_field_value ($column_name, $row_name, $player_nid, $editable, $field_width = 5) {
  module_load_include ('inc', 'stat');
  module_load_include ('inc', 'hockey_boss_stat');

  $stat_units = array ();
  foreach (stat_get_unit_terms () as $term) {
    $stat_units [$term->tid] = $term->name;
  }

  // I. Retrieve the stat associated with the field.
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
    $stat_unit     = stat_get_unit ($stat);
    $stat_unit_tid = stat_get_unit_tid ($stat);
    $stat_value    = stat_get_value ($stat);
    if ($editable) {
      return array (
        'value' => array (
          '#type' => 'container',
          "stat_value_$stat->nid" => array (
            '#type'               => 'textfield',
            '#default_value'      => "$stat_value",
            '#size'               => $field_width
          ),
          "stat_unit_$stat->nid" => array (
            '#type'              => 'select',
            '#options'           => $stat_units,
            '#default_value'     => $stat_unit_tid,
            '#empty_option'      => '- None -',
            '#required'          => false
      )));
    } else {
      return "$stat_value $stat_unit";
    }
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


  $stat_test = stat_create_stat (
    hockey_boss_stat_pps_type (),
    $column_name,
    '0',
    $field_stat_year . '-01-01T12:00:00',
    $player_nid
 );
/*
  watchdog (
    'hockey_boss_stat_table',
    '[hockey_boss_stat_table_pps_default_field_value] stat: <pre>@stat</pre>',
    array (
      '@stat' => print_r ($stat_test, true)
  ));
*/
  if ($editable) {
    return array (
      'value' => array (
        '#type' => 'container',
        "stat_value_$stat->nid" => array (
          '#type'          => 'textfield',
          '#default_value' => '0',
          '#size'          => $field_width
        ),
        "stat_unit_$stat->nid" => array (
          '#type'          => 'select',
          '#options'       => $stat_units,
          '#empty_option'  => '- None -',
          '#required'      => false
    )));
  } else {
    return '0';
  }
}

/**
  @brief Accepts a submitted Player Performance
    Stat table form and updates its stats.
*/
function hockey_boss_stat_table_pps_submit ($form, $form_state) {
  foreach ($form_state ['input'] as $key => $value) {
    if (strpos ($key, 'stat_value_') === 0) {
      $stat_nid = substr ($key, 11);
      $stat = node_load ($stat_nid);
      if (is_null ($stat)) {
        continue;
      }
      stat_set_value ($stat, $value);
      node_save ($stat);
    } elseif (strpos ($key, 'stat_unit_') === 0) {
      $stat_nid = substr ($key, 10);
      $stat = node_load ($stat_nid);
      if (is_null ($stat)) {
        continue;
      }
      if ($value === '') {
        stat_unset_unit_tid ($stat);
      } else {
        stat_set_unit_tid ($stat, $value);
      }
      node_save ($stat);
    }
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

  global $user;
  $editable = hockey_boss_stat_table_pps_access ($user);

  $data = array (
    'module'  => 'hockey_boss_stat_table',
    'rows'    => array ('2014', '2013'),
    'columns' => array (
      array (
        'column_name' => t ('Year'),
        'weight'      => 10
      ),
      array (
        'column_name' => t ('Goals'),
        'weight'      => 9
      ),
      array (
        'column_name' => t ('Assists'),
        'weight'      => 8
      ),
      array (
        'column_name' => t ('Shots'),
        'weight'      => 7
      ),
      array (
        'column_name' => t ('Shots On Goal'),
        'weight'      => 6
      ),
      array (
        'column_name' => t ('Shots Against'),
        'weight'      => 5
      ),
      array (
        'column_name' => t ('Shots Against On Goal'),
        'weight'      => 4
      ),
      array (
        'column_name' => t ('Penalties'),
        'weight'      => 3
      ),
      array (
        'column_name' => t ('Face-Offs'),
        'weight'      => 2
      ),
      array (
        'column_name' => t ('+/-'),
        'weight'      => 1
    )), 
    'submit'                        => $editable ? array ('hockey_boss_stat_table_pps_submit') : null,
    'fields'                        => array (),
    'sortable'                      => !$editable,
    'sort_column'                   => t ('Year'),
    'label_column_name'             => t ('Year'),
    'default_field_value_function'  => 'hockey_boss_stat_table_pps_default_field_value',
    'default_field_value_arguments' => array ($player->nid, $editable, 5)
  ); 
  $form = drupal_get_form ('simple_table_form', $data);
  return $form;
}

function hockey_boss_stat_table_pfs ($player) {
  module_load_include ('inc', 'hockey_boss');
  module_load_include ('inc', 'hockey_boss_stat');
  module_load_include ('inc', 'stat');

  $stats = stat_get_stats_by_subject (hockey_boss_stat_PFS_TYPE, $player->nid);

  global $user;
  $editable = hockey_boss_stat_table_pfs_access ($user);

  $data = array (
    'module'  => 'hockey_boss_stat_table',
    'rows'    => array ('2014', '2013'),
    'columns' => array (
      array (
        'column_name' => t ('Year'),
        'weight'      => 10
      ),
      array (
        'column_name' => t ('Circles Without Puck'),
        'weight'      => 9
      ),
      array (
        'column_name' => t ('Circles With Puck'),
        'weight'      => 8
      ),
      array (
        'column_name' => t ('Russian Circles Without Puck'),
        'weight'      => 7
      ),
      array (
        'column_name' => t ('Russian Circles With Puck'),
        'weight'      => 6
      ),
      array (
        'column_name' => t ('Full Ice Sprint'),
        'weight'      => 5
      ),
      array (
        'column_name' => t ('Suicides'),
        'weight'      => 4
      ),
      array (
        'column_name' => t ('Push-ups'),
        'weight'      => 3
      ),
      array (
        'column_name' => t ('Sit-ups'),
        'weight'      => 2
      ),
      array (
        'column_name' => t ('Pull-ups'),
        'weight'      => 1
      ),
      array (
        'column_name' => t ('Benchpress'),
        'weight'      => 0
      ),
      array (
        'column_name' => t ('Legpress'),
        'weight'      => -1
      ),
      array (
        'column_name' => t ('Squat'),
        'weight'      => -2
      ),
      array (
        'column_name' => t ('60 Yard Dash'),
        'weight'      => -3
      ),
      array (
        'column_name' => t ('Mile Time'), 
        'weight'      => -4
    )), 
    'submit'                        => $editable ? array ('hockey_boss_stat_table_pfs_submit') : null,
    'fields'                        => array (),
    'sortable'                      => !$editable,
    'sort_column'                   => t ('Year'),
    'label_column_name'             => t ('Year'),
    'default_field_value_function'  => 'hockey_boss_stat_table_pfs_default_field_value',
    'default_field_value_arguments' => array ($player->nid, $editable, 5)
  ); 
  $form = drupal_get_form ('simple_table_form', $data);
  return $form;
}
