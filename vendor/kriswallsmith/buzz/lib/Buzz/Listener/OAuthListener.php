<?php

namespace Buzz\Listener;

use Buzz\Util\Url;

use Buzz\Exception\ClientException;

use Buzz\Message\MessageInterface;
use Buzz\Message\RequestInterface;

class OAuthListener implements ListenerInterface
{
    /**
     * @var string
     */
    private $method;
    /**
     * @var array
     */
    private $options;

    /**
     * @param string $method
     * @param array  $options
     */
    public function __construct($method = 'AUTH_HTTP_TOKEN', array $options)
    {
        $this->method  = $method;
        $this->options = $options;
    }

    /**
     * {@inheritDoc}
     *
     * @throws InvalidArgumentException
     */
    public function preSend(RequestInterface $request)
    {
        // Skip by default
        if (null === $this->method) {
            return;
        }

        switch ($this->method) {

            case 'AUTH_HTTP_TOKEN':
                if (!isset($this->options['token'])) {
                    throw new ClientException('You need to set OAuth token!');
                }

                $request->addHeader('Authorization: token '. $this->options['token']);
                break;

            case 'AUTH_URL_CLIENT_ID':
                if (!isset($this->options['client_id'], $this->options['client_secret'])) {
                    throw new ClientException('You need to set client_id and client_secret!');
                }

                $this->setRequestUrl(
                    $request,
                    array(
                        'client_id'     => $this->options['client_id'],
                        'client_secret' => $this->options['client_secret'],
                    )
                );
                break;

            default:
                throw new ClientException(sprintf('Unknown method called "%s".', $this->method));
        }
    }

    /**
     * {@inheritDoc}
     */
    public function postSend(RequestInterface $request, MessageInterface $response)
    {
    }

    /**
     * @param RequestInterface $request
     * @param array            $parameters
     *
     * @return Url
     */
    private function setRequestUrl(RequestInterface $request, array $parameters = array())
    {
        $url  = $request->getUrl();
        $url .= (false === strpos($url, '?') ? '?' : '&').utf8_encode(http_build_query($parameters, '', '&'));

        $request->fromUrl(new Url($url));
    }
}