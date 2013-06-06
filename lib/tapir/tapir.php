<?php

class Tapir {
  private $auth;
  private $auth_opts;
  private $APIs;
  private $parameters;

  public function __construct($config) {
    $this->parameters = array();
    
    //load conf from file
    if (is_string($config)) {
      $filename = __DIR__ . '/api/' . $config . '.json';
      if (is_readable($filename) && ($file = file_get_contents($filename))) {
        $config = json_decode($file, TRUE);
        if (!$config) {
          throw new Exception('Could not load json file: ' . $file);
          return FALSE;
        }
      }
    }
    
    foreach ($config['apis'] as $api => $calls) {
      $this->APIs[$api] = new API($this, $calls);
    }
  }

  public function setParameters($params = array()) {
    $this->parameters = $params;
  }

  public function getParameters() {
    return $this->parameters;
  }

  //should this be a separate object?  feels like it could be overridable for different auth or transport options.
  public function useOAuth($consumer_key, $consumer_secret, $token, $secret) {
    if (isset($this->auth)) {
      throw new Exception('Authentication has already been set.');
    }

    require_once(__DIR__ . '/OAuth-PHP/OAuth.php');
    $this->auth = 'oauth';
    $this->auth_opts = array(
      'consumer' => new OAuthConsumer($consumer_key, $consumer_secret),
      'token' => new OAuthToken($token, $secret),
    );
  }

  public function buildQuery($method, $url, $parameters = array()) {
    if ($this->auth == 'oauth') {
      $request = OAuthRequest::from_consumer_and_token($this->auth_opts['consumer'], $this->auth_opts['token'], $method, $url);
      $request->sign_request(new OAuthSignatureMethod_PLAINTEXT(), $this->auth_opts['consumer'], $this->auth_opts['token']);

      foreach ($parameters as $key => $val) {
        $request->set_parameter($key, $val);
      }

      $return = $request->to_url();
    } else {
      //no auth specified.
      $return = $url . '?' . http_build_query($parameters);
    }

    return $return;

  }

  public function query($method, $url, $parameters = array()) {
    if (!isset($this->auth) || $this->auth == 'oauth') {
      $ch = curl_init();
      curl_setopt($ch, CURLOPT_URL, $url);
      curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);

      switch (strtolower($method)) {
        case 'put':
          curl_setopt($ch, CURLOPT_PUT, TRUE);
          //TRUE to HTTP PUT a file. The file to PUT must be set with CURLOPT_INFILE and CURLOPT_INFILESIZE.
          break;
        case 'patch':
          //if params is specified earlier it doesn't work.
          //maybe it must not be called as oath set param
          //$parameters = array('id' => '4', 'custom_fields' => array('github_issue_id' => '9'));
          $data = json_encode($parameters);
          print_r($data);
          print("\n");
          //$data = '{"custom_fields":{"github_issue_id":"4"}}';
          print_r($data);
          //print_r($parameters);
          //$data = array('subject'=>'another api subject patch');
          curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'PATCH' );
          curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: application/json', 'Accept: application/json'));
          curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
          //http://stackoverflow.com/questions/11532363/does-php-curl-support-patch
          break;
        case 'post':
          curl_setopt($ch, CURLOPT_POST, TRUE);

        case 'get':
          //default, do nothing
          break;
      }

      $data = curl_exec($ch);
      curl_close($ch);
      return ($data && $json = json_decode($data)) ? $json : FALSE;
    }
  }

  public function api($api) {
    if (isset($this->APIs[$api])) {
      return  $this->APIs[$api];
    } else {
      throw new Exception('Unregistered API: ' . $api);
    }
  }

}


class APICall {
  private $method = null;
  private $url; 
  private $post;

  public function __construct($call) {
    $this->method = $call['method'];
    $this->url = $call['url'];
    $this->post = (isset($call['post_parameters'])) ? $call['post_parameters'] : NULL;
  }

  public function method() { return $this->method; }

  public function url() { return $this->url; }

  public function parameters($parameters) {
    if ($this->post) {
      $use_params = array_combine($this->post, $this->post);
      return array_intersect_key($parameters, $use_params);
    } else {
      return array();
    }
  }

  public function query($parameters) {
    if ($this->post) {
      return array_diff_key($parameters, array_flip($this->post));
    }
  }
}


class API {
  private $APICaltapirSs;
  private $tapirService;

  public function __construct($tapirService, $calls) {
    $this->tapirService = $tapirService;
    $this->APICalls = array();
    foreach ($calls as $name => $call) {
      $this->addCall($name, $call);
    }
  }

  public function call($cmd, $parameters = array()) {
    $tapir = $this->tapirService;
    $parameters += $tapir->getParameters();

    if (!isset($this->APICalls[$cmd])) {
      throw new Exception('Call does not exist: ' . $cmd);
    }


    $call = $this->APICalls[$cmd];
    $url = $this->getUrl($call->url(), $parameters);
    $url = $tapir->buildQuery($call->method(), $url, $call->query(parameters));

    $result = $tapir->query($call->method(), $url, $call->parameters($parameters));
    return $result;
  }

  public function addCall($name, $call) {
    $this->APICalls[$name] = new APICall($call);
  }

  private function getUrl($url, &$parameters) {
    $pattern = '/{.*?}/';
    $tokens = array();
    preg_match_all($pattern, $url, $tokens);
    $tokens = preg_replace('/[{}]/', '', $tokens[0]); //strip {}

    foreach ($tokens as $token) {
      //$token = trim($token, '{}');
      if (!isset($parameters[$token])) {
        throw new Exception('Parameter error.  "'.$token.'" is required.');
      }
      $url = str_replace('{'.$token.'}', $parameters[$token], $url);
      unset($parameters[$token]);
    }

    return $url;
  }


}
