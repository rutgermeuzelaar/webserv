function deleteRequest(uri)
{
    console.log(uri);
    fetch(
        uri,
        {method: 'DELETE'}
    )
    .then(response => console.log(response))
    .then(() => window.location.reload())
}
