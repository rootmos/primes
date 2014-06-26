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

primesTMWE = [2,3,5,7] ++ _Y ((11:) . tail  . gapsW 11 wheel 
                                    . joinT . hitsW 11 wheel)
 
gapsW k (d:w) s@(c:cs) | k < c     = k : gapsW (k+d) w s
                       | otherwise =     gapsW (k+d) w cs      -- k==c
hitsW k (d:w) s@(p:ps) | k < p     =     hitsW (k+d) w s
                       | otherwise = scanl (\c d->c+p*d) (p*p) (d:w) 
                                       : hitsW (k+d) w ps      -- k==p 
wheel = 2:4:2:4:6:2:6:4:2:4:6:6:2:6:4:2:6:4:6:8:4:2:4:2:
        4:8:6:4:6:2:4:6:2:6:6:4:2:4:6:2:6:4:2:4:2:10:2:10:wheel

main = mapM_ print $ take 2000000 primesTMWE
