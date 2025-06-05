import Data.Array.MArray
import Data.Array.IO
import System.IO
import qualified Data.ByteString.Builder as B

data Picture = Picture
    { picWidth :: Int
    , picHeight :: Int
    , picArray :: IOUArray Int Float}

newPicture :: Int -> Int -> IO Picture
newPicture w h = do {
    arr <- newArray (0, w * h * 3) 0.0;
    return $ Picture w h arr;
}

setPixel :: (Int, Int) -> (Float, Float, Float) -> Picture -> IO ()
setPixel (x, y) (r, g, b) pic = let {
    idx = (y * picWidth pic + x) * 3;
} in do {
    writeArray (picArray pic) idx r;
    writeArray (picArray pic) (idx+1) g;
    writeArray (picArray pic) (idx+2) b;
}

outputPicture :: String -> Picture -> IO ()
outputPicture fileName pic = withBinaryFile fileName WriteMode $ \h -> let {
    fileLength = undefined;
    width = undefined;
    height = undefined;
    imageSize = undefined;
} in do {
    B.hPutBuilder h $ mconcat 
        [ -- header
          B.word16LE 0x4d42
        , B.word32LE fileLength
        , B.word32LE 0
        , B.word32LE 54
          -- info
        , B.word32LE 40
        , B.int32LE width
        , B.int32LE height
        , B.word16LE 1
        , B.word16LE 24
        , B.word32LE 0 -- BI_RGB
        , B.word32LE imageSize
        , B.word32LE 0
        , B.word32LE 0
        , B.word32LE 0
        , B.word32LE 0
        ];
}

