module Main where

import Data.Foldable (for_)
import Data.Maybe (fromMaybe)
import GHC.Eventlog.Socket
import System.Environment

main :: IO ()
main = do
    fibberEventlogSocket <-
        fromMaybe "/tmp/fibber_eventlog.sock"
            <$> lookupEnv "FIBBER_EVENTLOG_SOCKET"
    startWait fibberEventlogSocket
    args <- getArgs
    for_ args $ \arg ->
        print (fib (read arg))

fib :: Integer -> Integer
fib 0 = 0
fib 1 = 1
fib n = fib (n - 1) + fib (n - 2)
