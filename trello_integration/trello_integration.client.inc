<?php 

use Trello\Trello;

/**
 * @class OSTrello
 *
 * Extends php-trello's Trello class with helper fuctions.  Use these instead of raw URLs.
 * 
 * Examples:
 *   // Get all lists on a board
 *   $trello->boards($board_id)->lists()->get();
 *   
 *   // Create a new card (other params include desc, due, labels, idMembers)
 *   $card = array('idList' => $list_id, 'name' => 'My card\'s title')
 *   $trello->cards()->create($card);
 *   
 *   // Update labels on a card (overwrites current labels, does not append)
 *   $trello->cards($card_id)->labels()->update('red,green,blue');
 *   
 *   //Loop over all webhooks and delete them
 *   foreach ($trello->tokens($token)->webhooks()->get() as $hook) {
 *     $trello->webhooks($hook->id)->delete();
 *   }
 */
class OSTrello extends Trello {
  private $models;
  
  public function __construct($consumer_key, $shared_secret = null, $token = null, $oauth_secret = null) {
    parent::__construct($consumer_key, $shared_secret, $token, $oauth_secret);
    
    /**
     * This array describes the structure of Trello's rest URLs.  Top level items correspond to URLs.  They often take
     * and id parameter.  Their children can similarly be targetted.
     * 
     * So, to query a specific card, you'd use $trello->cards($id).  CRUD can be done there, or a subquery from the cards 
     * can be made: $trello->cards($id)->actions($comment_id)
     */
    $models = array(
      'actions' => array('board', 'card', 'list', 'member', 'memberCreator', 'organization'),
      'boards' => array('cards','lists','members', 'membersInvited', 'memberships', 'organization', 'labelNames', 'myPrefs', 'prefs', 'powerUps'),
      'cards' => array('field', 'attachments', 'board', 'list', 'actions' => array('comments'), 'checklist' => array('checkItem'), 'checklists', 'idMembers', 'labels', 'membersVoted',),  
      'checklists' => array('field', 'board', 'cards', 'checkItems'),
      'lists' => array('field', 'board','cards'),
      'members' => array('field', 'board', 'cards', 'notifications', 'organizations', 'organizationsInvited', 'prefs'),
      'notifications' => array('field', 'board', 'cards', 'member', 'memberCreator', 'organization'),
      'organizations' => array('field', 'attachments', 'boards', 'members', 'membersInvited', 'memberships', 'prefs'),
      'search' => array('members'),
      'tokens' => array('member','webhooks'),
      'webhooks' => array('field'),  
    );

    foreach ($models as $key => $value) {
      $url = (is_array($value)) ? $key : $value;
      $children = (is_array($value)) ? $value : array();
      $this->models[$url] = new Model($this, $url, $children);
    }
  }
  
  public function __call($method, $arguments) {
    
    //call one of our models as a method
    if (isset($this->models[$method])) {
      return $this->models[$method]->args($arguments);
    } else {
      $response = parent::__call($method, $arguments);
      if (!$response) {
        drupal_set_message($this->error(), 'error');
      }
      return $response;
    }
  }
}


class Model {
  private $client;
  private $url;
  private $models;
  private $args;
  private $parent;
  
  function __construct(OSTrello $client, $url, $children = array(), $parent = NULL) {
    $this->client = $client;
    $this->url = $url;
    $this->parent = $parent;
    
    foreach ($children as $key => $value) {
      $url = (is_array($value)) ? $key : $value;
      $grandchildren = (is_array($value)) ? $value : array();
      $this->models[$url] = new Model($client, $url, $grandchildren, $this);
      
      //$this->models[$child] = new Model($client, $child, $grandchildren, $this);
    }
  }
  
  public function __call($method, $arguments) {  
    if (isset($this->models[$method])) {
      return $this->models[$method]->args($arguments);
    } 
  }
  
  function args(array $args) {
    $this->args = $args;
    return $this;
  }
  
  private function url() {
    $url = ($this->parent) ? $this->parent->url() . '/' : '';
    return $url . $this->url . '/' . implode('', $this->args);
  }
  
  function get($id = NULL, $params = array()) {
    $url = ($id) ? $this->url() . '/' . $id : $this->url();
    return $this->client->get($url, $params);
  }
  function create(array $params) {
    $url = $this->url();
    return $this->client->post($url, $params);
  }

  function update($id, $params = array()) {
    if (empty($params)) {
      $params = array('value' => $id);
      $id = NULL;
    }
    
    $url = $this->url() . $id;
    return $this->client->put($url, $params);
  }
  function delete($id = NULL) {
    return $this->client->delete($this->url() . $id);
  }
}