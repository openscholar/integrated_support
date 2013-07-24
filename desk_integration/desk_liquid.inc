<?php

/**
 * @function desk_liquid_remplate
 *
 * Returns liquid template for json payload of the desk.com vars we want
 */
function desk_liquid_template() {
//just hardcode it for now
  $string_vars = array(
    "customer_name" => "{{customer.name}}",
    "case_id" => "{{case.id}}",
    "case_subject" => "{{case.subject}}",
    "case_labels" => "{{case.labels}}",
    "case_user" => "{{case.user.name}}",
    "case_custom_github_issue_id" => "{{case.custom_github_issue_id}}",

    "os" => "{{case.interactions | map: 'os' | first}}",
    "os_version" => "{{case.interactions | map: 'os_version' | first}}",
    "browser" => "{{case.interactions | map: 'browser' | first}}",
    "browser_version" => "{{case.interactions | map: 'browser_version' | first}}",
  );

  $array_vars = array(
    "case_body" => "[{{case.interactions | map: 'body' | escape: | join: '\",\"' | append: '\"' | prepend: '\"' }}]",
  );


  $fields = array();
  foreach ($string_vars as $field => $template) {
    $fields[] = "\"$field\":\"$template\"";
  }

  foreach ($array_vars as $field => $template) {
    $fields[] = "\"$field\":$template";
  }

  return '{' . implode(',', $fields) . '}';
}

function desk_liquid_url_template() {
  //generates ['customer_name' => '{{customer.name}}']
  $merged = array();
  foreach (desk_liquid_vars() as $k => $v) {
   $merged[] = "$k=$v"; 
  }
  //return http_build_query($ret); //escaped the curly braces and broke liquid
  return implode('&', $merged);
}

function desk_liquid_vars() {
  $fields = array('customer.name', 'case.id', 'case.subject', 'case.labels', 'case.user.name', 'case.custom_github_issue_id', 'case.user.id', 'case.customer.id');
  $ret = array_combine(
    str_replace('.', '_', $fields), 
    preg_replace('/.*/', '{{$0}}', $fields, 1)
  );

  $ret += array(
    "os" => "{{case.interactions | map: 'os' | first}}",
    "os_version" => "{{case.interactions | map: 'os_version' | first}}",
    "browser" => "{{case.interactions | map: 'browser' | first}}",
    "browser_version" => "{{case.interactions | map: 'browser_version' | first}}",
    "case_body" => "{{case.interactions | map: 'body' | first | replace: '\"' \"'\" | append: '\"' | prepend: '\"' }}",
  );
  
  return $ret;
}

