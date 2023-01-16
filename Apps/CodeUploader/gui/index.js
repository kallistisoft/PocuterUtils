const $ = ( id ) => ( document.getElementById( id ) );

function waiting( state ) {
    console.log(`waiting(${state})`);
    if( state ) {
        document.body.style.cursor =
        $('dropzone').style.cursor = 
        $('button').style.cursor = 'wait';
    } else {
        document.body.style.cursor =
        $('dropzone').style.cursor = 
        $('button').style.cursor = null;
    }
}

let imageFile = null;
let is_uploading = false;
function UploadFile() {

    // test: not currently uploading a file
    if( is_uploading ) {
        console.error("UploadFile() - called while in use!");
        return;
    }

    // verify: image file exists
    if( imageFile == null ) {
        waiting( false );

        console.error("UploadFile() - called with non-existant imageFile!");
        return;
    }

    // verify: image file size is reasonable
    if( imageFile.size < 600*1024 ) {
        imageFile == null;
        waiting( false );

        alert("Error image file must be >= 600KiB in size!");
        return;
    }

    // set: uploading state busy flags
    console.log(`Upload: ${imageFile.name}`);
    is_uploading = true;
    waiting( true );

    // update: display element text
    $('image_appid').innerText = imageFile.appid;
    $('image_path').innerText = './' + imageFile.fullpath;
    $('image_size').innerText = `${(imageFile.size / 1024).toFixed(2)} Kib`;
    $('image_md5sum').innerText = imageFile.md5sum;
    $('app_name').innerText = imageFile.appName;

    // update: display element attributes
    $('dropzone').setAttribute('loaded','true');    
    $('progress_bar').style.display = 'block';
    $('button').setAttribute('enabled','true');
    $('button').style.display = 'none';

    // create: parameters object
    const params = new FormData();    
    params.append( 'appID', imageFile.appid );
    params.append( 'appMD5', imageFile.md5sum );    
    params.append( 'appSize', imageFile.size );
    params.append( 'appImage', imageFile );

    // create: xhr request object
    const xreq = new XMLHttpRequest();

    // onprogress: update progress bar
    xreq.upload.onprogress = ( event ) => {
        if (event.lengthComputable) {
            const percentage = Math.round((event.loaded * 100) / event.total);
            $('progress_bar').value = percentage;
            console.log('progress: ',percentage);
        }
    }

    // onloadend: clear wait state flags and 
    xreq.onloadend = () => {
        if( xreq.status === 200 ) {
            console.log( xreq.responseText );            
            console.log('Done uploading file...');
            alert( xreq.responseText );
        }
        waiting( false );
        is_uploading = false;

        $('progress_bar').value = 0;
        $('progress_bar').style.display = 'none';
        $('button').style.display = 'block';
    };

    // Initiate a multipart/form-data upload
    xreq.open( "POST", '/upload', true );
    xreq.send( params );
}

window.onload = () => {
    console.log("window.onload()");

    const dropzone = $('dropzone');  

    window.addEventListener('drop', (event) => { 
        event.preventDefault(); 
    });

    window.addEventListener('dragover', (event) => { 
        event.dataTransfer.dropEffect = 'none';
        event.preventDefault(); 
    });

    // set: display dragover attribute
    dropzone.ondragenter = dropzone.ondragover = (event) => { 
        event.stopPropagation();
        event.preventDefault();
        if( !event.target == dropzone ) return;
        dropzone.setAttribute('dragover','true');
    }

    // reset: display dragover attribute
    document.body.ondragenter = (event) => { 
        event.stopPropagation();
        event.preventDefault();
        if( !event.target == dropzone ) return;
        dropzone.setAttribute('dragover','false');
    }

    dropzone.ondrop = (event) => {

        // clear: drag event state
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        event.stopPropagation();
        event.preventDefault();

        // set: element CSS state attributes
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        dropzone.setAttribute('dragover','false');
        dropzone.setAttribute('loaded','false');
        $('button').setAttribute('enabled','false');        
        
        // reset imageFile and control states
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        imageFile = null;
        $('image_md5sum').innerText = "";        
        $('image_appid').innerText = "";
        $('image_path').innerText = "";
        $('image_size').innerText = "";
        $('app_name').innerText = "";

        // test that only one folder was dropped
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        const filesArray = event.dataTransfer.files;
        if( filesArray.length > 1 ) {
            alert("Only one file may be dragged and dropped at a time!");
            return;
        }
        
        waiting(true);

        // traverseFileTree(item, path = "") :: recursively walk file tree
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // TODO: limit depth recursion to five folders, add relevant error message
        let is_traversing = false;
        function traverseFileTree(item, path = "", depth = 0 ) {
            path = path || "";
            
            if( depth >= 5 ) {
                console.error(`traverseFileTree(...) ${depth} limit reached`);                
                return;
            }

            // exit: already found image file
            if( imageFile != null ) return;

            // file: parse full path name and check for appid folder
            if (item.isFile) {
                item.file(function(file) {
                    let appid = parseInt( path.split('/').reverse()[1] );
                    if( isNaN( appid ) || appid < 2 ) return;
                    if( file.name === 'esp32c3.app' ) {
                        imageFile = file;
                        imageFile.fullpath = path + file.name;
                        imageFile.appid = appid;
                        console.log("id:", imageFile.appid );
                        console.log("path:", imageFile.fullpath );
                    }
                });
            } 

            // dir: iterate sub-folders and recurse
            else if (item.isDirectory) {
                let dirReader = item.createReader();
                dirReader.readEntries(function(entries) {
                    for (let i=0; i<entries.length; i++) {
                        traverseFileTree(entries[i], path + item.name + "/", depth + 1 );
                    }
                });
            }

            if( !depth ) is_traversing = false;
        }

        // get transfer item and walk file tree
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        let items = event.dataTransfer.items;
        let item = items[0].webkitGetAsEntry();
        if (item) {
            is_traversing = true;
            traverseFileTree( item );
        }

        // ugly hack to get around async behavior of directory transversal
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        let timer = window.setInterval( () => {

            if( is_traversing ) return;
            window.clearInterval( timer );

            if( imageFile == null ) {
                alert("Could not find Pocuter image file 'esp32c3.app'!");
                waiting(false);
                return;
            }

            let reader = new FileReader();            
            reader.onload = function(event) {
                let wordArray = CryptoJS.lib.WordArray.create(this.result);
                let md5 = CryptoJS.MD5(wordArray).toString();
                imageFile.md5sum = md5;
                
                // extract: image file header
                let head = this.result.slice(48,512);
                    head = String.fromCharCode.apply(null, new Uint8Array(head));                

                // extract: app name from image file header
                let matches = head.match(/Name=([\w|\s]+)\r\n/i);
                if( !matches ) matches = head.match(/Name=([\w|\s]+)\n/i);
                imageFile.appName = ( matches ? matches[1] : '' );
                
                // log: image data to console
                console.log('md5:', imageFile.md5sum );
                console.log('size:', imageFile.size );
                console.log( 'name:', imageFile.appName);

                // upload: processed image file
                UploadFile();
            };
            reader.readAsArrayBuffer( imageFile );
        }, 200 );
    };
}