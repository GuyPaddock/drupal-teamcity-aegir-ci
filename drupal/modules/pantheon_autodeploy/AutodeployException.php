<?php
/**
 * Parent type for exceptions that occur when Auto-deploy is performing an
 * operation.
 */
class AutodeployException extends Exception {
  private $drushErrorCode;

  /**
   * Constructor for <code>AutodeployException</code>.
   *
   * @param string $code
   *   The Drush-friendly error code.
   * @param string $message
   *   The human-friendly error message.
   * @param Exception|null $previous
   *   The previous exception used for the exception chaining.
   */
  public function __construct($drushErrorCode, $message, Exception $previous = null) {
    parent::__construct($message, 0, $previous);

    $this->drushErrorCode = $drushErrorCode;
  }

  /**
   * Gets the error code that can be passed to <code>drush_set_error()</code>.
   *
   * @return string
   *   The Drush error code for this exception.
   */
  public function getDrushErrorCode() {
    return $this->drushErrorCode;
  }
}
