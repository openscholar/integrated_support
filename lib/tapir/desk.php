<?php

/*
class DeskTapir extends Tapir {

  public function __construct($subdomain) {
    $this->url = 'http://' . $subdomain . '/desk.com/';
    $this->addCall('case', '')
    $this->apis = array(
      'case' => new API('method', 'url', 'required')
    );
  }
  private function buildQuery($url, $method = 'get', $parameters = array()) {
    $paramters +=...
      parent::
  }
}
 */
$desk_api = array(
  'case' => array(
    'list' => array(
      'url' => 'https://{subdomain}.desk.com/api/v2/cases',
      'method' => 'get',
    ),
    'search' => array(
      'url' => 'https://{subdomain}.desk.com/api/v2/cases/search',
      'method' => 'get',
    ),
    'show' => array(
      'url' => 'https://{subdomain}.desk.com/api/v2/cases/{id}',
      'method' => 'get',
    ),
    'update' => array(
      'url' => 'https://{subdomain}.desk.com/api/v2/cases/{id}',
      'method' => 'patch',
    ),
  )
);

require_once('tapir.php');
$desk_token_secret='';
$desk_token='';
$desk_consumer_key='';
$desk_consumer_secret='';


$desk = new Tapir($desk_api);
$desk->setParameters(array('subdomain' => 'jsagotsky'));
$desk->useOAuth($desk_consumer_key, $desk_consumer_secret, $desk_token, $desk_token_secret);
$api = $desk->api('case');
//print_r($api);


//$result = $api->call('search', array('case_id' => 4));
$result = $api->call('list');
print_r($result);



//goal:
// $desk = new deskTapir();
// $desk->api('case')->call('show', array('id' => $id))
