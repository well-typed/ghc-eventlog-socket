{-# LANGUAGE ForeignFunctionInterface #-}

-- |
-- Stream GHC eventlog events to external processes.
module GHC.Eventlog.Socket
    ( startWait, start
    ) where

-- | Start listening for eventlog connections, blocking until a client connects.
startWait :: IO ()
startWait = c_start True

-- | Start listening for eventlog connections.
start :: IO ()
start = c_start False

foreign import ccall safe "eventlog_socket_start" c_start :: Bool -> IO ()
