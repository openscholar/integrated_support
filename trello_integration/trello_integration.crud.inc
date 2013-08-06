<?php 

/**
 * @file trello_integration.crud.inc
 */
/**
 * @function trello_integration_create_card()
 * 
 * Creates a trello card on the specified list
 */
function trello_integration_create_card($list, $name, $desc = NULL, $pos = 'bottom', $due = NULL, $labels = array(), $members = array()) {
  $card = array(
    'idList' => $list,
    'name' => $name,
    'desc' => $desc,
    'due' => $due,
    'labels' => array_intersect($labels, array('green', 'yellow', 'orange', 'red', 'purple', 'blue')),
    'idMembers' => $members, 
  );

  $trello = trello_integration_client();
  $trello->post('cards', array_filter($card));
}

function trello_integration_lists($board) {
  $trello = trello_integration_client();
  return $trello->boards->get("$board/lists");
}

function trello_integration_cards($list) {
  $trello = trello_integration_client();
  return $trello->lists->get("$list/cards");
}