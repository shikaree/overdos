const http=require('http'),fs=require('fs'),path=require('path');
http.createServer((req,res)=>{
  let f=req.url==='/'?'/overdos.html':req.url.split('?')[0];
  f=path.join(__dirname,decodeURIComponent(f));
  fs.readFile(f,(e,d)=>{ if(e){res.writeHead(404);res.end('nf');return;}
    res.writeHead(200,{'Content-Type':f.endsWith('.html')?'text/html':'application/octet-stream'});res.end(d);});
}).listen(8099,()=>console.log('serving http://localhost:8099'));
