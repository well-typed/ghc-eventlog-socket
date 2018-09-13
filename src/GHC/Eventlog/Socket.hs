{-# LANGUAGE ForeignFunctionInterface #-}

module GHC.Eventlog.Socket
    ( start
    ) where

-- | Start listening for eventlog connections, blocking until a client connects.
foreign import ccall unsafe "eventlog_socket_start" start :: IO ()
