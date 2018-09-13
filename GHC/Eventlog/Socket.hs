{-# LANGUAGE ForeignFunctionInterface #-}

module GHC.Eventlog.Socket
    ( start
    ) where

foreign import ccall "eventlog_socket_start" start :: IO ()
