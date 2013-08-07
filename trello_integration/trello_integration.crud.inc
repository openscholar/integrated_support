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


function trello_integration_webhooks() {
  $trello = trello_integration_client();
  $token = variable_get('trello_integration_token');
  return $trello->get('tokens/' . $token . '/webhooks');
}

function trello_integration_delete_webhook($id) {
  trello_integration_client()->delete('webhooks/' . $id);
}
  
function trello_integration_create_webhook($id, $callback_url, $description = '') {
  $trello = trello_integration_client();
  $token = variable_get('trello_integration_token');
  $key = variable_get('trello_integration_key');
  $webhook = array(
    'idModel' => $id,
    'callbackURL' => $callback_url,
    'description' => $description,    
  );
  
  $post = $trello->post('webhooks', array_filter($webhook));
  if (!$post) {
    //'not a valid url' refers to the callbackURL.
    drupal_set_message(t('Error creating webhook to "@url": @e', array('@url' => $callback_url, '@e' => $trello->error())), 'error');
  }
  
  return $post;
}

function trello_integration_card_comments($card) {
  $trello = trello_integration_client();
  
  //we can bail out in advance if whole card is provided
  if (is_object($card) && !$card->badges->comments) {
    return NULL;
  }
  
  $card_id = (is_object($card)) ? $card->id : $card;
  return $trello->cards->get($card->id . '/actions');
}

function trello_integration_card($id) {
  $trello = trello_integration_client();
  return $trello->cards->get($id);
}