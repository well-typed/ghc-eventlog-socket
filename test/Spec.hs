module Main where

import GHC.Eventlog.Socket
import Test.QuickCheck

main :: IO ()
main = do
  -- this creates the unix socket
  startWait (Just "/tmp/eventlog.sock") -- if this were `startWait Nothing` you'd get the value from the GHC_EVENTLOG_SOCKET environment variable
  quickCheck $ withMaxSuccess 100 prop_associative_float

prop_associative_float :: Float -> Float -> Float -> Bool
prop_associative_float x y z = ((x * y) * z) == (x * (y * z))
