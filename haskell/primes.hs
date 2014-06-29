import System.IO
-- import Data.Foldable
import Control.Monad


_Y   :: (t -> t) -> t
_Y g = g (_Y g)

minus (x:xs) (y:ys) = case (compare x y) of 
           LT -> x : minus  xs  (y:ys)
           EQ ->     minus  xs     ys 
           GT ->     minus (x:xs)  ys
minus  xs     _     = xs
union (x:xs) (y:ys) = case (compare x y) of 
           LT -> x : union  xs  (y:ys)
           EQ -> x : union  xs     ys 
           GT -> y : union (x:xs)  ys
union  xs     []    = xs
union  []     ys    = ys

primesTME = 2 : _Y ((3:) . gaps 5 . joinT . map (\p-> [p*p, p*p+2*p..]))


joinT ((x:xs):t) = x : (union xs . joinT . pairs) t  -- ~= nub.sort.concat
  where  pairs (xs:ys:t) = union xs ys : pairs t 
 
gaps k s@(x:xs) | k<x  = k:gaps (k+2) s    -- ~= [k,k+2..]\\s, when
                | True =   gaps (k+2) xs   --   k<=x && null(s\\[k,k+2..])


-- Another one

primesTMWE = [3,7] ++ _Y ((11:) . tail  . gapsW 11 wheel2 
                                    . joinT . hitsW 11 wheel2)
 
gapsW k (d:w) s@(c:cs) | k < c     = k : gapsW (k+d) w s
                       | otherwise =     gapsW (k+d) w cs      -- k==c
hitsW k (d:w) s@(p:ps) | k < p     =     hitsW (k+d) w s
                       | otherwise = scanl (\c d->c+p*d) (p*p) (d:w) 
                                       : hitsW (k+d) w ps      -- k==p 

wheel2 = 2:4:2:4:6:2:6:4:2:4:6:6:2:6:4:2:6:4:6:8:4:2:4:2:
        4:8:6:4:6:2:4:6:2:6:6:4:2:4:6:2:6:4:2:4:2:10:2:10:wheel2
--

factors = take 750 primesTMWE


not_divisible :: Int -> Int -> Bool
not_divisible a b = mod a b /= 0 

divisible :: Int -> Int -> Bool
divisible a b = mod a b == 0 

factor_test :: Int -> [Bool]
factor_test a = map (not_divisible a) factors

is_prime :: Int -> Bool
is_prime a = not $ any (divisible a) factors

start = 5697
end = 32452843
--wheel = 2:2:2:4:wheel -- ..7+wheel=..9,.11,..13,17
--wheel = 2:2:2:4:2:2:2:4:2:2:6:wheel
wheel = 4:2:4:2:4:4:2:2:6:wheel
numbers = dropWhile (<start) ([3,7,11] ++ (scanl (+) 11 wheel2))

print_if_prime file x = when (is_prime x) $ hPutStrLn file (show x)
        
main = do
    file <- openFile "list" WriteMode
    mapM_ (hPutStrLn file . show) factors
    mapM_ (print_if_prime file) check
    hClose file
    where
        check = takeWhile (<=end) numbers 
