{-# LANGUAGE ForeignFunctionInterface #-}

-- |
-- Stream GHC eventlog events to external processes.
module GHC.Eventlog.Socket
    ( startWait, start
    ) where

import Foreign.C
import Foreign.Ptr

-- | Start listening for eventlog connections, blocking until a client connects.
startWait :: Maybe FilePath
          -- ^ File path to the unix domain socket to create. If @Nothing@, then
          -- the @GHC_EVENTLOG_SOCKET@ environment variable is used.
          -> IO ()
startWait = c_start' True

-- | Start listening for eventlog connections.
start :: Maybe FilePath
      -- ^ File path to the unix domain socket to create. If @Nothing@, then
      -- the @GHC_EVENTLOG_SOCKET@ environment variable is used.
      -> IO ()
start = c_start' False

c_start' :: Bool -> Maybe FilePath -> IO ()
c_start' block socketPathMay = case socketPathMay of
    Nothing -> c_start nullPtr block
    Just socketPath -> withCString socketPath $ \socketPathCString ->
                            c_start socketPathCString block

foreign import ccall safe "eventlog_socket_start" c_start :: CString -> Bool -> IO ()
