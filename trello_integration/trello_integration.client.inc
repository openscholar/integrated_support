<?php 

use Trello\Trello;

class OSTrello extends Trello {
  private $models;
  
  public function __construct($consumer_key, $shared_secret = null, $token = null, $oauth_secret = null) {
    parent::__construct($consumer_key, $shared_secret, $token, $oauth_secret);
    $models = array(
      'actions' => array(
        'board' => array(),
        'card' => array(),
        'list' => array(),
        'member' => array(),
        'memberCreator' => array(),
        'organization' => array(),
      ),
      
      'boards' => array(
        'cards' => array(),
        'lists' => array(),
        'members' => array(),
        'membersInvited' => array(),
        'memberships' => array(),
        'organization' => array(),
        'labelNames' => array(),
        'myPrefs' => array(),
        'prefs' => array(),
        'powerUps' => array(),
      ),
        
      'cards' => array(
        'field' => array(),
        'attachments' => array(),
        'board' => array(),
        'list' => array(),
        'actions' => array(
          'comments' => array(),    
        ),
        'checklist' => array(
          'checkItem' => array(),    
        ),
        'checklists' => array(),
        'idMembers' => array(),
        'labels' => array(),
        'membersVoted' => array(),    
      ),
        
      'checklists' => array(
        'field' => array(),
        'board' => array(),
        'cards' => array(),
        'checkItems' => array(),    
      ),
        
      'lists' => array(
        'field' => array(),
        'board' => array(),
        'cards' => array(),
      ),
        
      'members' => array(
        'field' => array(),
        'board' => array(),
        'cards' => array(),          
        'notifications' => array(),
        'organizations' => array(),
        'organizationsInvited' => array(),
        'prefs' => array(),
      ),

      'notifications' => array(
        'field' => array(),
        'board' => array(),
        'cards' => array(),
        'member' => array(),
        'memberCreator' => array(),
        'organization' => array(),
      ),
        
      'organizations' => array(
        'field' => array(),
        'attachments' => array(),
        'boards' => array(),
        'members' => array(),
        'membersInvited' => array(),
        'memberships' => array(),
        'prefs' => array(),
      ),
      'search' => array(
        'members' => array(),    
      ),
      'tokens' => array(
        'member' => array(),
        'webhooks' => array(),
      ),
      'webhooks' => array(
        'field' => array(),    
      ),
    );
    foreach ($models as $url => $children) {
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

//how to do: $client->boards()->members->get
// GET /1/boards/[board_id]/members
// GET /1/boards/[board_id]/members/[filter]
// GET /1/boards/[board_id]/members/[idMember]/cards
//
// longest:
// /1/cards/[card id or shortlink]/checklist/[idChecklist]/checkItem/[idCheckItem]/convertToCard
// $client->cards($id)->checklist(id)->checkItem(id)->convertToCard

class Model {
  private $client;
  private $url;
  private $models;
  private $args;
  private $parent;
  
  function __construct(OSTrello $client, $url, $children, $parent = NULL) {
    $this->client = $client;
    $this->url = $url;
    $this->parent = $parent;
    
    foreach ($children as $child => $grandchildren) {
      $this->models[$child] = new Model($client, $child, $grandchildren, $this);
    }
  }
  
  public function __call($method, $arguments) {  
    //call one of our models as a method
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
  function update($id, $params) {}
  function delete($id, $params) {}
}



