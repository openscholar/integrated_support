<?php

function trello_integration_webhook_handler() {
  //no cache for you!  varnish should never cache this page.
  drupal_add_http_header('Cache-Control', 'no-store, no-cache, must-revalidate');
  drupal_add_http_header('Pragma', 'no-cache');

  $json = @file_get_contents('php://input');
  $payload = json_decode($json);
  $action = $payload->action;
  watchdog('trello', "Trello recieved a @type webhook:<br />@hook", array('@hook' => $json, '@type' => $payload->action->type));

  switch ($action->type) {
    // card was created in Trello
    case 'createCard':
      // check that this is a brand new card, and not one added via REST from GS or GH
      if (1) {
        // allow other services to react to this event
        $data = $action->data->card;
        $card = array(
          'title' => $data->name,
          'trello_id' => $data->id,
          'list' => $action->data->list->id,
          'milestone' => $action->data->list->name,
          'desc' => '',
        );
        module_invoke_all('trello_webhook', 'createCard', $card);
      }
      break;

    case 'updateCard':
      switch (key($action->data->old)) {
        // card was moved to a different list
        case 'idList':
          // check that the new list's title is a milestone or 'Pending Review'
          $data = $action->data->card;
          if (1) {
            // allow other services to react to this event
            $card = array(
              'trello_id' => $data->id,
              'list' => $action->data->list->id,
              'milestone' => $action->data->listAfter->name,
            );
            module_invoke_all('trello_webhook', 'updateCardMilestone', $card);
          }
          break;
        // card's description was changed
        case 'desc':
          // allow other services to react to this event
          $data = $action->data->card;
          $card = array(
            'trello_id' => $data->id,
            'description' => $data->desc,
          );
          module_invoke_all('trello_webhook', 'updateCardDescription', $card);
          break;
      }
      break;
    // card was deleted from Trello
    case 'deleteCard':
        $data = $action->data->card;
        $card = array(
          'trello_id',
        );
        module_invoke_all('trello_webhook', 'deleteCard', $card);
        integrated_support_delete_link('trello', $data->id);
      break;
    // new List was created
    case 'createList':
      break;
    case 'updateList':
      switch (key($action->old)) {
        // name change
        case 'name':
          break;
        // archived
        case 'closed':
          break;
      }
      break;
  }
}