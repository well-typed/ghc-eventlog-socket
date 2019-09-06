{-# LANGUAGE ForeignFunctionInterface #-}

-- |
-- Stream GHC eventlog events to external processes.
module GHC.Eventlog.Socket
    ( start
    ) where

-- | Start listening for eventlog connections, blocking until a client connects.
foreign import ccall unsafe "eventlog_socket_start" start :: IO ()
