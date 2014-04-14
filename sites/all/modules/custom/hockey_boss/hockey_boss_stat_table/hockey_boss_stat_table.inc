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

  $field_stat_name = $column_name;
  $field_stat_year = $row_name;

  $stats = stat_get_stats_by_subject (hockey_boss_stat_PPS_TYPE, $player_nid);
  foreach ($stats as $stat) {
    if ($stat->title != $field_stat_name) {
      continue;
    }
    $stat_year = stat_get_year ($stat);
    if ($stat_year != $field_stat_year) {
      continue;
    }
    $stat_value    = stat_get_value ($stat);
    if ($editable) {
      return array (
        'value' => array (
          '#type' => 'container',
          "stat_value_$stat->nid" => array (
            '#type'               => 'textfield',
            '#default_value'      => $stat_value,
            '#size'               => $field_width
      )));
    } else {
      // return "$stat_value $stat_unit";
      return "$stat_value";
    }
  }
  // II. Create a stat for undefined fields.
  $stat_date = simple_field_create_date_field_value ($field_stat_year, 0, 0);
  $stat_date = $stat_date ['value'];
  $stat_type = hockey_boss_stat_pps_type ();
  $stat = stat_create_stat_node ($stat_type, $field_stat_name, '', '', $stat_date, $player_nid);

  if ($editable) {
    return array (
      'value' => array (
        '#type' => 'container',
        "stat_value_$stat->nid" => array (
          '#type'          => 'textfield',
          '#size'          => $field_width
    )));
  } else {
    return '';
  }
}

/**
  @brief Accepts a submitted Player Performance
    Stat table form and updates its stats.
*/
function hockey_boss_stat_table_pps_submit ($form, $form_state) {
  module_load_include ('inc', 'stat');
  foreach ($form_state ['input'] as $key => $value) {
    if (strpos ($key, 'stat_value_') === 0) {
      $stat_nid = substr ($key, 11);
      $stat = node_load ($stat_nid);
      if (is_null ($stat)) {
        continue;
      }
      stat_set_value ($stat, $value);
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

  $start_year = '2012';
  $current_year = date ('Y');
  $years = array ();
  for ($year = $start_year; $year <= $current_year; $year ++) {
    $years [] = $year;
  }

  $data = array (
    'module'  => 'hockey_boss_stat_table',
    'rows'    => $years,
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

/**
  @brief The access callback function for the
    Player Game Stats table.
*/
function hockey_boss_stat_table_pgs_access ($user) {
  return ($user->uid == 1);
}

/**
  @brief The default field value callback function
    for the Player Game Stats table.
*/
function hockey_boss_stat_table_pgs_default_field_value ($column_name, $row_name, $player_nid, $editable, $field_width = 5) {
  module_load_include ('inc', 'stat');
  module_load_include ('inc', 'hockey_boss');
  module_load_include ('inc', 'hockey_boss_stat');

  $field_stat_name = $column_name;
  $field_stat_game_nid  = $row_name;
  $game_node = node_load ($field_stat_game_nid);
  if (!$game_node) {
    return '';
  }

  if ($column_name == t ('Game')) {
    return $game_node->title;
  }
  if ($column_name == t ('Date')) {
    $game_date = date_create (hockey_boss_get_game_date ($game_node));
    return date_format ($game_date, 'Y-m-d');
  }

  $stats = stat_get_stats_by_subject (hockey_boss_stat_PGS_TYPE, $player_nid);
  foreach ($stats as $stat) {
    if ($stat->title != $field_stat_name) {
      continue;
    }
    $stat_game_nid = hockey_boss_stat_pgs_get_game ($stat);
    if ($stat_game_nid != $field_stat_game_nid) {
      continue;
    }
    $stat_value = stat_get_value ($stat);
    if ($editable) {
      return array (
        'value' => array (
          '#type' => 'container',
          "stat_value_$stat->nid" => array (
            '#type'               => 'textfield',
            '#default_value'      => $stat_value,
            '#size'               => $field_width
      )));
    } else {
      return $stat_value;
    } 
  }
  $stat_date = $game_node->{$game_node->type . '_date'}['und'][0]['value']; 
  $stat_type = hockey_boss_stat_pgs_type ();
  $stat = stat_create_stat_node ($stat_type, $field_stat_name, '', '', $stat_date, $player_nid);
  hockey_boss_stat_pgs_set_game ($stat, $game_node->nid);
  node_save ($stat);
  watchdog (
    'hockey_boss_stat_table',
    '[hockey_boss_stat_table_pgs_default_field_value] new stat: <pre>@stat</pre>',
    array (
      '@stat' => print_r ($stat, true)
  ));
  if ($editable) {
    return array (
      'value' => array (
        '#type' => 'container',
        "stat_value_$stat->nid" => array (
          '#type'          => 'textfield',
          '#size'          => $field_width
    )));
  } else {
    return '';
  }
}

/**
  @brief Accepts a submitted Player Game Stats
    table and updates its stats.
*/
function hockey_boss_stat_table_pgs_submit ($form, $form_state) {
  module_load_include ('inc', 'stat');
  foreach ($form_state ['input'] as $key => $value) {
    if (strpos ($key, 'stat_value_') === 0) {
      $stat_nid = substr ($key, 11);
      $stat = node_load ($stat_nid);
      if (is_null ($stat)) {
        continue;
      }
      stat_set_value ($stat, $value);
      node_save ($stat);
    }
  }
  drupal_set_message ('Stats updated.');
}

/**
  @brief Accepts a player node and returns a simple
    table form that represents the player's game
    stats table.
  @param $player (node) the player node.
  @return (array) the stats table form.
*/
function hockey_boss_stat_table_pgs ($player) {
  module_load_include ('inc', 'hockey_boss');
  module_load_include ('inc', 'hockey_boss_stat');
  module_load_include ('inc', 'stat');

  $stats = stat_get_stats_by_subject (hockey_boss_stat_PGS_TYPE, $player->nid);

  global $user;
  $editable = hockey_boss_stat_table_pgs_access ($user);

  // get the games that the player's team have played in.
  $player_team_nid = hockey_boss_get_player_team ($player);
  $game_nids = array ();
  foreach (hockey_boss_get_game_nodes () as $game) {
    $game_team_nids = hockey_boss_get_game_teams ($game);
    if ($player_team_nid == $game_team_nids [0] ||
        $player_team_nid == $game_team_nids [1]) {
      $game_nids [] = $game->nid;
    }
  } 
  $data = array (
    'module'  => 'hockey_boss_stat_table',
    'rows'    => $game_nids,
    'columns' => array (
      array (
        'column_name' => t ('Game'),
        'weight'      => 11
      ),
      array (
        'column_name' => t ('Date'),
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
    'submit'                        => $editable ? array ('hockey_boss_stat_table_pgs_submit') : null,
    'fields'                        => array (),
    'sortable'                      => !$editable,
    'sort_column'                   => t ('Date'),
    'default_field_value_function'  => 'hockey_boss_stat_table_pgs_default_field_value',
    'default_field_value_arguments' => array ($player->nid, $editable, 5)
  ); 
  $form = drupal_get_form ('simple_table_form', $data);
  return $form;
}

/**
  @brief the Access callback function for the team
    roster table.
*/
function hockey_boss_stat_table_tr_access ($user) {
  return ($user->$uid == 1);
}

/**
  @brief The default field value callback function
    for the Team Roster table.
*/
function hockey_boss_stat_table_tr_default_field_value ($column_name, $row_name, $team_nid, $editable, $field_width = 5) {
  module_load_include ('inc', 'stat');
  module_load_include ('inc', 'hockey_boss_stat');

  $field_stat_name = $column_name;
  $field_stat_player_nid = $row_name;
}
